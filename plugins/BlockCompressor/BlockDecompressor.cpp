#include <BlockDecompressor.h>

int main(int argc, char ** argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: ./BlockDecompressor <config_file> <matrix> <ef_path> <output>\n\n";
        exit(2);
    }

    //Initialize decompressor and decompressor each blocks to <output>
    BlockDecompressor(argv[1], argv[2], argv[3]).decompress_all(argv[4]);
}

BlockDecompressor::BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressor(ConfigurationLiterate(config_path), matrix_path, ef_path) {}

BlockDecompressor::BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path)
{
    this->config = ConfigurationLiterate(config);
    matrix.open(matrix_path, std::ifstream::binary);

    //serialize ef from ef_path
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");

    filters[0] = { .id = LZMA_FILTER_LZMA1 , .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN,  .options = NULL }; //Terminal filter
    
    line_size = ((config.get_nb_samples() + 7) / 8);
    block_decoded_size = line_size * config.get_lines_per_block();

    out_buffer.resize(block_decoded_size);
    
    ef_in.open(ef_path, std::ifstream::binary);

    //Deserialize vector
    std::size_t ef_pos_size;
    ef_in.read(reinterpret_cast<char*>(&ef_pos_size), sizeof(std::size_t)); //Retrieve size

    ef_pos.resize(ef_pos_size);
    ef_in.read(reinterpret_cast<char*>(ef_pos.data()), ef_pos_size * sizeof(std::uint64_t));
}

void BlockDecompressor::assert_lzma_ret(lzma_ret code)
{
    switch(code)
    {
        case LZMA_OK:
        case LZMA_STREAM_END:
            return;
        case LZMA_PROG_ERROR:
            throw std::runtime_error("Some parameters may be invalid");
        case LZMA_BUF_ERROR:
            throw std::runtime_error("Not enough memory space allocated for buffer");
        case LZMA_MEM_ERROR:
            throw std::runtime_error("Not enough memory space on machine");
        default:
            throw std::runtime_error("LZMA return code not handled");
    }
}


std::size_t BlockDecompressor::decode_block(std::size_t i)
{
    //Next code in comment shoudn't be written because public members ensures that ef_pos[i+1] exists
    /*if(i + 1 >= ef_pos.size())
        throw std::runtime_error("Attempted to read a block that doesn't exist (i=" + std::to_string(i) + ")");*/

    //Retrieve from EF encoding the location of corresponding block and its size
    std::size_t pos_a = ef_pos[i];
    std::size_t pos_b = ef_pos[i+1]; 

    std::size_t block_encoded_size = pos_b - pos_a;

    //If buffer current size doesn't match block size, resize it
    //May add a supplementary integer to store the maximum buffer size needed, to avoid multiple possible resizing
    if(block_encoded_size > in_buffer.size())
        in_buffer.resize(block_encoded_size);

    matrix.seekg(pos_a); //Seek block location
    matrix.read(reinterpret_cast<char*>(in_buffer.data()), block_encoded_size); //Read block

    std::size_t decoded_size = 0;
    std::size_t zero = 0;

    lzma_ret code = lzma_raw_buffer_decode(filters, NULL, in_buffer.data(), &zero, block_encoded_size, out_buffer.data(), &decoded_size, block_decoded_size);
    assert_lzma_ret(code);

    //Check if decoded size match expected size (taking into account the possibility of a smaller last block)
    if(decoded_size != block_decoded_size && (i + 2 != ef_pos.size()))
        throw std::runtime_error("Decoded block got an unexpected size: " + std::to_string(decoded_size) + " (should have been " + std::to_string(block_decoded_size) + ")");

    return decoded_size;
}

const std::uint8_t* BlockDecompressor::get_bit_vector_from_hash(std::uint64_t hash)
{
    //Get block index
    std::uint64_t block_index = hash / config.get_lines_per_block();
    //Get index in block
    std::uint64_t hash_index = hash % config.get_lines_per_block();

    if(block_index + 1 >= ef_pos.size()) //Handle queried hashes that are out of matrix
        return nullptr;
        
    if(block_index != decoded_block_index) //Avoid decompressing a block that was decompressed on last call
    {
        read_once = true;
        decode_block(block_index);
    }

    //Return corresponding bit_vector
    return out_buffer.data() + hash_index * get_line_size();
}

void BlockDecompressor::decompress_all(const std::string& out_path)
{
    std::ofstream out_file;
    out_file.open(out_path, std::ofstream::binary);
    std::size_t decoded_size;

    //i < ef_pos.size() - 1 but as it is unsigned, avoid possible underflows if ef_pos is empty
    for(std::size_t i = 0; i + 1 < ef_pos.size(); ++i)
    {
        decoded_size = decode_block(i);
        out_file.write(reinterpret_cast<const char*>(out_buffer.data()), decoded_size);
    }
}

std::uint64_t BlockDecompressor::get_line_size() const
{
    return line_size;
}
