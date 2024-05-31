#include <BlockCompressor.h>

void BlockCompressor::assert_lzma_ret(lzma_ret code)
{
    switch(code)
    {
        case LZMA_OK:
        case LZMA_STREAM_END:
            return;
        case LZMA_PROG_ERROR:
            throw std::runtime_error("LZMA: Some parameters may be invalid");
        case LZMA_BUF_ERROR:
            throw std::runtime_error("LZMA: Not enough memory space allocated for buffer");
        case LZMA_MEM_ERROR:
            throw std::runtime_error("LZMA: Not enough memory space on machine");
        default:
            throw std::runtime_error("LZMA: Return code not handled");
    }
}

void BlockCompressor::write_buffer()
{
    std::size_t in_size = in_buffer_current_size ? in_buffer_current_size : in_buffer.size();
    std::size_t out_size = 0;

    //Let default allocator by passing NULL (malloc/free)
    lzma_ret code = lzma_raw_buffer_encode(filters, NULL, in_buffer.data(), in_size, out_buffer.data(), &out_size, out_buffer.size());
    assert_lzma_ret(code);

    //Add position to Elias-Fano encoder
    current_size += out_size;
    ef_pos.push_back(current_size);

    //Write bytes to output file
    m_out.write(reinterpret_cast<const char*>(out_buffer.data()), out_size * sizeof(std::uint8_t));
    m_out.flush();
}

BlockCompressor::BlockCompressor()
{
    ef_pos.push_back(0);
}

BlockCompressor::~BlockCompressor()
{
    //std::cout << "a: " << std::to_string(m_partition) << ": " << bit_vectors_read << std::endl;

    //Check for a possible underflow that would add a huge amount of buffers of zeroes
    if(maximum_hash != previous_hash)
       fill_zero_buffers(maximum_hash - previous_hash - 1); //Take into account last possible empty bit_vectors to be added to blocks

    //std::cout << "b: " << std::to_string(m_partition) << ": " << bit_vectors_read << std::endl;
    //Write a smaller block if buffer isn't empty
    if(in_buffer_current_size != 0)
        write_buffer();

    //Elias-Fano encoding
    write_elias_fano();

    //std::cout << "c: " << std::to_string(m_partition) << ": " << bit_vectors_read << std::endl;
    ef_out.close();
    m_out.close();
}

void BlockCompressor::write_elias_fano()
{
    //https://github.com/ot/succinct

    //ef_builder = succint::elias_fano::elias_fano_builder(ef_pos.size(), ef_pos.back());
    std::size_t ef_size = ef_pos.size();
    ef_out.write(reinterpret_cast<const char*>(&ef_size), sizeof(std::size_t));
    ef_out.write(reinterpret_cast<const char*>(ef_pos.data()), sizeof(std::uint64_t) * ef_pos.size());
}

void BlockCompressor::fill_zero_buffers(std::uint64_t n)
{
    if(n >= 1)
    {
        std::fill(m_buffer.begin(), m_buffer.end(), 0);
        for(std::uint64_t i = 0; i < n; ++i)
            add_buffer_to_block();
    }
}

void BlockCompressor::add_buffer_to_block(const std::uint8_t * bit_vector)
{
    //Fill vector starting from its "current size"
    if(bit_vector == nullptr)
        std::memcpy(in_buffer.data()+in_buffer_current_size, m_buffer.data(), m_buffer.size());
    else
        std::memcpy(in_buffer.data()+in_buffer_current_size, bit_vector, m_buffer.size());
    
    ++bit_vectors_read;

    // Update variables tracking data to use in in_buffer vector
    in_buffer_current_size = m_buffer.size() * (bit_vectors_read % config.get_bit_vectors_per_block());

    if(bit_vectors_read % config.get_bit_vectors_per_block() == 0)
    {
        // Compress block
        write_buffer();
    }
}

// 'hash' is the hash value
// 'counts' is the abundance vector of the line, not binarized.
// Note that you may not see all the lines here, as 'process_hash' is not called on empty lines, i.e. hashes not present in any samples. I don't know how you want to handle this, in the non compressed index, I simply write empty bit vectors.
bool BlockCompressor::process_hash(std::uint64_t hash, std::vector<count_type>& counts)
{
    if(hash > maximum_hash)
        throw std::runtime_error("Hash was greater than maximum allowed, '" + std::to_string(hash) + "' > '" + std::to_string(maximum_hash) + "'");

    if(hash < minimum_hash)
        throw std::runtime_error("Hash was lesser than minimum allowed, '" + std::to_string(hash) + "' < '" + std::to_string(minimum_hash) + "'");

    if(hash == previous_hash && hash != minimum_hash)
        throw std::runtime_error("Hash was equal to previous hash received '" + std::to_string(hash) + "' == '" + std::to_string(previous_hash) + "'");
    
    if(hash != previous_hash)
    {
        if(hash < previous_hash)
            throw std::runtime_error("Unexpected hash, got '" + std::to_string(hash) + "' which is less than previous hash '" + std::to_string(previous_hash) + "' (min: '" + std::to_string(minimum_hash) + "' , maximum: '" + std::to_string(maximum_hash) + "')");

        std::uint64_t diff = hash - previous_hash - 1;

        // Add missing hashes
        fill_zero_buffers(diff);
    }

    previous_hash = hash;

    km::set_bit_vector(m_buffer, counts); // binarize counts into m_buffer
    add_buffer_to_block();

    return false; // You manage output yourself, tell kmtricks to not write the line.
}

void BlockCompressor::process_binarized_bit_vector(std::uint64_t hash, const std::uint8_t * bit_vector)
{
    if(hash > maximum_hash)
        throw std::runtime_error("Hash was greater than maximum allowed, '" + std::to_string(hash) + "' > '" + std::to_string(maximum_hash) + "'");

    if(hash < minimum_hash)
        throw std::runtime_error("Hash was lesser than minimum allowed, '" + std::to_string(hash) + "' < '" + std::to_string(minimum_hash) + "'");

    if(hash == previous_hash && hash != minimum_hash)
        throw std::runtime_error("Hash was equal to previous hash received '" + std::to_string(hash) + "' == '" + std::to_string(previous_hash) + "'");
    
    if(hash != previous_hash)
    {
        if(hash < previous_hash)
            throw std::runtime_error("Unexpected hash, got '" + std::to_string(hash) + "' which is less than previous hash '" + std::to_string(previous_hash) + "' (min: '" + std::to_string(minimum_hash) + "' , maximum: '" + std::to_string(maximum_hash) + "')");

        std::uint64_t diff = hash - previous_hash - 1;

        // Add missing hashes
        fill_zero_buffers(diff);
    }

    previous_hash = hash;
    add_buffer_to_block(bit_vector);
}

// 'config_path' is the string passed to --plugin-config, path to the config file
void BlockCompressor::configure(const std::string& config_path)
{
    //Parse configuration file
    //Assignments must be specified this way:
    //  PROPERTY1 = VALUE
    //  PROPERTY2 = VALUE
    //  ...
    config.load(config_path);

    minimum_hash = previous_hash = km::HashWindow(this->m_output_directory + "/../hash.info").get_lower(this->m_partition);
    maximum_hash = km::HashWindow(this->m_output_directory + "/../hash.info").get_upper(this->m_partition);

    //Configure buffer size according to parameters
    m_buffer.resize((config.get_nb_samples() + 7) / 8);
    std::string part_str = std::to_string(this->m_partition);

    std::string output_path = this->m_output_directory + "/../matrices/matrix_" + part_str;
    std::string ef_path = this->m_output_directory + "/../matrices/ef_" + part_str;

    m_out.open(output_path, std::ofstream::binary);
    ef_out.open(ef_path, std::ofstream::binary);

    //Configure options and filters (compression level) 
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");

    std::uint32_t BLOCK_DECODED_SIZE = m_buffer.size() * config.get_bit_vectors_per_block();
    opt_lzma.dict_size = MAX_DICT_SIZE < BLOCK_DECODED_SIZE ? BLOCK_DECODED_SIZE : MAX_DICT_SIZE;

    filters[0] = { .id = LZMA_FILTER_LZMA1, .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN, .options = NULL }; //Terminal filter

    in_buffer.resize(m_buffer.size() * config.get_bit_vectors_per_block());

    //Compression is not inplace, so we need to allocate out_buffer once for storing data
    //Get maximum estimated (upper bound) encoded size
    out_buffer.resize(lzma_stream_buffer_bound(in_buffer.size()));
}

void BlockCompressor::no_plugin_configure(const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned partition)
{
    config.load(config_path);

    minimum_hash = previous_hash = km::HashWindow(hash_info_path).get_lower(partition);
    maximum_hash = km::HashWindow(hash_info_path).get_upper(partition);

    /*std::cout << minimum_hash << std::endl;
    std::cout << maximum_hash << std::endl;*/

    //Configure buffer size according to parameters
    m_buffer.resize((config.get_nb_samples() + 7) / 8);
    std::string part_str = std::to_string(partition);

    std::string output_path = out_prefix + part_str;
    std::string ef_path = out_prefix + "_ef_" + part_str;

    m_out.open(output_path, std::ofstream::binary);
    ef_out.open(ef_path, std::ofstream::binary);

    //Configure options and filters (compression level) 
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");

    std::uint32_t BLOCK_DECODED_SIZE = m_buffer.size() * config.get_bit_vectors_per_block();
    opt_lzma.dict_size = MAX_DICT_SIZE < BLOCK_DECODED_SIZE ? BLOCK_DECODED_SIZE : MAX_DICT_SIZE;

    filters[0] = { .id = LZMA_FILTER_LZMA1, .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN, .options = NULL }; //Terminal filter

    in_buffer.resize(m_buffer.size() * config.get_bit_vectors_per_block());

    //Compression is not inplace, so we need to allocate out_buffer once for storing data
    //Get maximum estimated (upper bound) encoded size
    out_buffer.resize(lzma_stream_buffer_bound(in_buffer.size()));
}

int BlockCompressor::get_partition_from_filename(const std::string& filename)
{
    int a = -1, b = filename.size();
    while(b >= 0)
    {
        if(filename[b] == '.')
        {
            a = b = b - 1;
            break;
        }

        --b;
    }

    while(a >= 0)
    {
        if(filename[a] < '0' || filename[a] > '9')
        {
            a = a + 1;
            break;
        }

        --a;
    }

    return std::stoi(filename.substr(a, b-a+1));
}

void BlockCompressor::compress_pa_hash(const std::string& in_path, const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned short skip_header)
{
    BlockCompressor b;

    //Parse partition number using input filename '^[0-9]*[0-9]*\..*'
    
    int partition = get_partition_from_filename(in_path);

    b.no_plugin_configure(out_prefix, config_path, hash_info_path, partition);

    std::ifstream in_file(in_path, std::ifstream::binary);

    in_file.seekg(0, std::ifstream::end);
    std::uint64_t size = in_file.tellg() - (long)skip_header;
    in_file.seekg(skip_header, std::ifstream::beg);

    const std::uint64_t BIT_VECTOR_SIZE = b.m_buffer.size();
    const std::uint64_t PA_HASH_LINE_SIZE = (sizeof(std::uint64_t) + BIT_VECTOR_SIZE);

    if(size % PA_HASH_LINE_SIZE != 0)
        throw std::runtime_error("File size doesn't match [<hash> <bit_vector>], check the header size");

    const std::uint64_t NB_HASH_BV = size / PA_HASH_LINE_SIZE;
    
    std::uint8_t * bit_vector = new std::uint8_t[BIT_VECTOR_SIZE];

    std::uint64_t hash;
    for(std::uint64_t i = 0; i < NB_HASH_BV; ++i)
    {
        in_file.read(reinterpret_cast<char*>(&hash), sizeof(std::uint64_t));
        in_file.read(reinterpret_cast<char*>(bit_vector), BIT_VECTOR_SIZE);
        b.process_binarized_bit_vector(hash, bit_vector);
    }
    
    delete[] bit_vector;
}

void BlockCompressor::compress_cmbf(const std::string& in_path, const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned short skip_header)
{
    BlockCompressor b;
    
    //Parse partition number using input filename '^[0-9]*[0-9]*\..*'
    
    int partition = get_partition_from_filename(in_path);

    b.no_plugin_configure(out_prefix, config_path, hash_info_path, partition);

    std::ifstream in_file(in_path, std::ifstream::binary);
    
    in_file.seekg(0, std::ifstream::end);
    std::uint64_t size = in_file.tellg() - (long)skip_header;
    in_file.seekg(skip_header, std::ifstream::beg);

    const std::uint64_t CMBF_LINE_SIZE = b.m_buffer.size();
    const std::uint64_t CMBF_BLOCK_SIZE = b.in_buffer.size();

    if(size % CMBF_LINE_SIZE != 0)
        throw std::runtime_error("File size doesn't match [<bit_vector>], check the header size");

    //Write full blocks
    const std::uint64_t NB_FULL_BLOCKS = size / CMBF_BLOCK_SIZE;

    b.in_buffer_current_size = 0;

    for(std::uint64_t i = 0; i < NB_FULL_BLOCKS; ++i)
    {
        in_file.read(reinterpret_cast<char*>(b.in_buffer.data()), CMBF_BLOCK_SIZE);
        b.write_buffer();
    }

    //Write last block
    if(size % CMBF_BLOCK_SIZE != 0)
    {
        //Preset buffer tracking variable
        b.in_buffer_current_size = size % CMBF_BLOCK_SIZE; //Remaining bit_vectors

        in_file.read(reinterpret_cast<char*>(b.in_buffer.data()), b.in_buffer_current_size);
        b.write_buffer();
    }

    //Prevent plugin from writing data on destruction (excepted Elias-Fano)
    b.in_buffer_current_size = 0;
    b.previous_hash = b.maximum_hash;
}


extern "C" std::string plugin_name() { return "BlockCompressor"; }
extern "C" int use_template() { return 0; }
extern "C" km::IMergePlugin* create0() { return new BlockCompressor(); }
extern "C" void destroy(km::IMergePlugin* p) { delete p; }
