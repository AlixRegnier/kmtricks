#include <BlockCompressorZSTD.h>

std::size_t BlockCompressorZSTD::compress_buffer(std::size_t in_size)
{
    return ZSTD_compress2(context, out_buffer.data(), out_buffer.size(), in_buffer.data(), in_size);
}

//Init ZSTD filters and resize output buffer according to estimated compressed block size
void BlockCompressorZSTD::init_compressor()
{
    //Configure options and filters (compression level) 
    context = ZSTD_createCCtx();
    
    ZSTD_CCtx_setParameter(context, ZSTD_c_compressionLevel, ZSTD_maxCLevel());

    //Compression is not inplace, so we need to allocate out_buffer once for storing data
    //Get maximum estimated (upper bound) encoded size
    out_buffer.resize(ZSTD_compressBound(in_buffer.size()));
}


BlockCompressorZSTD::~BlockCompressorZSTD()
{
    //Free memory
    ZSTD_freeCCtx(context);
}

extern "C" std::string plugin_name() { return "BlockCompressorZSTD"; }
extern "C" int use_template() { return 0; }
extern "C" km::IMergePlugin* create0() { return new BlockCompressorZSTD(); }
extern "C" void destroy(km::IMergePlugin* p) { delete p; }
