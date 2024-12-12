#include <BlockDecompressorLZMA.h>

BlockDecompressorLZMA::BlockDecompressorLZMA(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size) : BlockDecompressorLZMA(ConfigurationLiterate(config_path), matrix_path, ef_path, header_size) {}

BlockDecompressorLZMA::BlockDecompressorLZMA(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size) : BlockDecompressor(config, matrix_path, ef_path, header_size)
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

std::size_t BlockDecompressorLZMA::decompress_buffer(std::size_t in_size)
{
    std::size_t zero = 0; //Must be set to 0 before using lzma_raw_buffer_encode()
    std::size_t written_bytes = 0; //Must be set to 0 before using lzma_raw_buffer_encode()

    lzma_ret code = lzma_raw_buffer_decode(filters, NULL, in_buffer.data(), &zero, in_size, out_buffer.data(), &written_bytes, BLOCK_DECODED_SIZE);
    assert_lzma_ret(code);

    return written_bytes;
}