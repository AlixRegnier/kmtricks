#include <BlockDecompressorZSTD.h>

BlockDecompressorZSTD::BlockDecompressorZSTD(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressorZSTD(ConfigurationLiterate(config_path), matrix_path, ef_path) {}

BlockDecompressorZSTD::BlockDecompressorZSTD(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path) : BlockDecompressor(config, matrix_path, ef_path)
{
    context = ZSTD_createDCtx();
    
    //Init options
    //No options are needed to be initialized when decompressing with Zstd
}


std::size_t BlockDecompressorZSTD::decompress_buffer(std::size_t in_size)
{
    return ZSTD_decompressDCtx(context, out_buffer.data(), out_buffer.size(), in_buffer.data(), in_size);
}

BlockDecompressorZSTD::~BlockDecompressorZSTD()
{
    ZSTD_freeDCtx(context);
}