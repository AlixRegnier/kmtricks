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

//May be used later
std::uint8_t BlockCompressor::padding4(std::size_t size)
{
    /* Equivalent code with if-else statement
    *
    * x = size % 4;
    * 
    * // x=0 => 0
    * // x=1 => 3
    * // x=2 => 2
    * // x=3 => 1
    * 
    * if(size % 4 == 0)
    *     return size + 0;
    * else
    *     return size + 4 - (size % 4);
    */

    std::size_t a = (size >> 1) & 0x1;
    std::size_t b = size & 0x1;

    return size + (((a ^ b) << 1) | b);
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
    if(in_buffer_current_size != 0)
        write_buffer();

    //Elias-Fano encoding
    write_elias_fano();

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

    std::cout << "Blocks written: " << (ef_size - 1) << std::endl;
}

void BlockCompressor::add_buffer_to_block()
{
    //Fill vector starting from its "current size"
    for(std::size_t i = 0; i < m_buffer.size(); ++i)
        in_buffer[in_buffer_current_size + i] = m_buffer[i];

    ++lines_read;

    // Update variables tracking data to use in in_buffer vector
    in_buffer_current_size = m_buffer.size() * (lines_read % config.get_lines_per_block());

    if(lines_read % config.get_lines_per_block() == 0)
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
    if(hash != previous_hash)
    {
        if(hash < previous_hash)
            throw std::runtime_error("Unexpected hash, got '" + std::to_string(hash) + "' which is less than previous hash '" + std::to_string(previous_hash) + "'");

        std::uint64_t diff = hash - previous_hash - 1;

        // Add missing hashes
        if(diff >= 1)
        {
            std::fill(m_buffer.begin(), m_buffer.end(), 0);
            for(std::uint64_t i = 0; i < diff; ++i)
                add_buffer_to_block();
        }
    }

    previous_hash = hash;
    km::set_bit_vector(m_buffer, counts); // binarize counts into m_buffer
    add_buffer_to_block();

    return false; // You manage output yourself, tell kmtricks to not write the line.
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

    filters[0] = { .id = LZMA_FILTER_LZMA1, .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN, .options = NULL }; //Terminal filter

    in_buffer.resize(m_buffer.size() * config.get_lines_per_block());

    //Compression is not inplace, so we need to allocate out_buffer once for storing data
    //Get maximum estimated (upper bound) encoded size
    out_buffer.resize(lzma_stream_buffer_bound(in_buffer.size()));
}

extern "C" std::string plugin_name() { return "BlockCompressor"; }
extern "C" int use_template() { return 0; }
extern "C" km::IMergePlugin* create0() { return new BlockCompressor(); }
extern "C" void destroy(km::IMergePlugin* p) { delete p; }
