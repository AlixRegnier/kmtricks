#include <BlockDecompressorLZMA.h>

BlockDecompressorLZMA::BlockDecompressorLZMA(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressorLZMA(ConfigurationLiterate(config_path), matrix_path, ef_path) {}

BlockDecompressorLZMA::BlockDecompressorLZMA(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressor(config, matrix_path, ef_path)
{
    //Set LZMA settings
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");

    //Use perfect-fitting dictionary size or maximum value if too large
    opt_lzma.dict_size = MAX_DICT_SIZE < BLOCK_DECODED_SIZE ? BLOCK_DECODED_SIZE : MAX_DICT_SIZE;

    filters[0] = { .id = LZMA_FILTER_LZMA1 , .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN, .options = NULL }; //Terminal filter
}

void BlockDecompressorLZMA::assert_lzma_ret(lzma_ret code)
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

void BlockDecompressor::decompress_buffer()
{
    std::size_t zero = 0; //Must be set to 0 before using lzma_raw_buffer_encode()
    decoded_block_size = 0; //Must be set to 0 before using lzma_raw_buffer_encode()

    lzma_ret code = lzma_raw_buffer_decode(filters, NULL, in_buffer.data(), &zero, block_encoded_size, out_buffer.data(), &decoded_block_size, BLOCK_DECODED_SIZE);
    assert_lzma_ret(code);
}