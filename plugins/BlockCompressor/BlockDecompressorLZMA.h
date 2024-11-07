#ifndef BLOCKDECOMPRESSORLZMA_H
#define BLOCKDECOMPRESSORLZMA_H

#include <BlockDecompressor.h>

#include <lzma.h>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressorLZMA : public BlockDecompressor
{
    private:
        //LZMA options        
        lzma_filter filters[2]; //Filters used by XZ
        lzma_options_lzma opt_lzma; //Options used in XZ filters

        //Check if a LZMA returned code is not a bad code
        static void assert_lzma_ret(lzma_ret code);

        void decompress_buffer() override;
    public:
        BlockDecompressorLZMA(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path);

        BlockDecompressorLZMA(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path);
};

#endif