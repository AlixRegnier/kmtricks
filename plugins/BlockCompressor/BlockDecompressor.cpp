#include <BlockDecompressor.h>

BlockDecompressor::BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressor(ConfigurationLiterate(config_path), matrix_path, ef_path) {}

BlockDecompressor::BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path)
{
    this->config = ConfigurationLiterate(config);
    matrix.open(matrix_path, std::ifstream::binary);

    //serialize ef from ef_path
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");
    
    bit_vector_size = ((config.get_nb_samples() + 7) / 8);
    BLOCK_DECODED_SIZE = bit_vector_size * config.get_bit_vectors_per_block();
    opt_lzma.dict_size = MAX_DICT_SIZE < BLOCK_DECODED_SIZE ? BLOCK_DECODED_SIZE : MAX_DICT_SIZE;

    filters[0] = { .id = LZMA_FILTER_LZMA1 , .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN,  .options = NULL }; //Terminal filter

    out_buffer.resize(BLOCK_DECODED_SIZE);
    
    ef_in.open(ef_path, std::ifstream::binary);

    //Deserialize size + EF
    ef_in.read(reinterpret_cast<char*>(&ef_size), sizeof(std::size_t)); //Retrieve size
    ef.load(ef_in);
    sdsl::util::init_support(ef_pos, &ef);
}

void BlockDecompressor::assert_lzma_ret(lzma_ret code)
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

void BlockDecompressor::unload()
{
    read_once = false;
}

void BlockDecompressor::decode_block(std::size_t i)
{
    //Retrieve from EF encoding the location of corresponding block and its size
    std::size_t pos_a = ef_pos(i+1);
    std::size_t pos_b = ef_pos(i+2); 

    std::size_t block_encoded_size = pos_b - pos_a;

    //If buffer current size doesn't match block size, resize it
    //May add a supplementary integer to store the maximum buffer size needed, to avoid multiple possible resizing
    if(block_encoded_size > in_buffer.size())
        in_buffer.resize(block_encoded_size);

    matrix.seekg(pos_a); //Seek block location
    matrix.read(reinterpret_cast<char*>(in_buffer.data()), block_encoded_size); //Read block

    std::size_t zero = 0;
    decoded_block_size = 0;

    lzma_ret code = lzma_raw_buffer_decode(filters, NULL, in_buffer.data(), &zero, block_encoded_size, out_buffer.data(), &decoded_block_size, BLOCK_DECODED_SIZE);
    assert_lzma_ret(code);

    //Check if decoded size match expected size (taking into account the possibility of a smaller last block)
    if(decoded_block_size != BLOCK_DECODED_SIZE && (i + 2 != ef_size))
        throw std::runtime_error("Decoded block got an unexpected size: " + std::to_string(decoded_block_size) + " (should have been " + std::to_string(BLOCK_DECODED_SIZE) + ")");

    decoded_block_index = i;
}

const std::uint8_t* BlockDecompressor::get_bit_vector_from_hash(std::uint64_t hash)
{
    //Need to retrieve <minimum_hash> to calculate from hash range starting from zero
    //if(hash < minimum_hash)
    //  throw std::runtime_error("<hash> was less than <minimum_hash>");
    //hash -= minimum_hash;

    //Get block index
    std::uint64_t block_index = hash / config.get_bit_vectors_per_block();
    //Get index in block
    std::uint64_t hash_index = hash % config.get_bit_vectors_per_block();

    if(block_index + 1 >= ef_size) //Handle queried hashes that are out of matrix
        return nullptr;

    if(read_once && hash_index >= decoded_block_size / get_bit_vector_size()) //Handle last block out index
        return nullptr;

    if(block_index != decoded_block_index || !read_once) //Avoid decompressing a block that was decompressed on last call
    {
        read_once = true;
        decode_block(block_index);
    }

    //Return corresponding bit_vector
    return out_buffer.data() + hash_index * get_bit_vector_size();
}

void BlockDecompressor::decompress_all(const std::string& out_path)
{
    std::ofstream out_file;
    out_file.open(out_path, std::ofstream::binary);

    //i < ef_size - 1 but as it is unsigned, avoid possible underflows if ef_pos is empty
    for(std::size_t i = 0; i + 1 < ef_size; ++i)
    {
        decode_block(i);
        out_file.write(reinterpret_cast<const char*>(out_buffer.data()), decoded_block_size);
    }
}

std::uint64_t BlockDecompressor::get_bit_vector_size() const
{
    return bit_vector_size;
}
