#include <BlockCompressorLZMA.h>

void BlockCompressorLZMA::assert_lzma_ret(lzma_ret code)
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

std::size_t BlockCompressorLZMA::compress_buffer(std::size_t in_size) override
{
    std::size_t out_size = 0;

    //Let default allocator by passing NULL (malloc/free)
    lzma_ret code = lzma_raw_buffer_encode(filters, NULL, in_buffer.data(), in_size, out_buffer.data(), &out_size, out_buffer.size());
    assert_lzma_ret(code);

    return out_size;
}

//Init LZMA filters and resize output buffer according to estimated compressed block size
void BlockCompressorLZMA::init_compressor() override
{
    //Configure options and filters (compression level) 
    if (lzma_lzma_preset(&opt_lzma, config.get_preset_level()))
        throw std::runtime_error("LZMA preset failed");

    std::uint32_t BLOCK_DECODED_SIZE = m_buffer.size() * config.get_bit_vectors_per_block();
    
    //Use perfect-fitting dictionary size or maximum value if too large
    opt_lzma.dict_size = MAX_DICT_SIZE < BLOCK_DECODED_SIZE ? BLOCK_DECODED_SIZE : MAX_DICT_SIZE;

    filters[0] = { .id = LZMA_FILTER_LZMA1, .options = &opt_lzma }; //Raw encoding with no headers
    filters[1] = { .id = LZMA_VLI_UNKNOWN, .options = NULL }; //Terminal filter

    //Compression is not inplace, so we need to allocate out_buffer once for storing data
    //Get maximum estimated (upper bound) encoded size
    out_buffer.resize(lzma_stream_buffer_bound(in_buffer.size()));
}


extern "C" std::string plugin_name() { return "BlockCompressorLZMA"; }
extern "C" int use_template() { return 0; }
extern "C" km::IMergePlugin* create0() { return new BlockCompressor(); }
extern "C" void destroy(km::IMergePlugin* p) { delete p; }
