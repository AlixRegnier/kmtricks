#ifndef BLOCKCOMPRESSORLZMA_H
#define BLOCKCOMPRESSORLZMA_H

#include <BlockCompressor.h>

//XZ utils main header
#include <lzma.h>

class BlockCompressorLZMA : public BlockCompressor
{
    private:
        //LZMA options
        lzma_options_lzma opt_lzma;
        lzma_filter filters[2];
    
        //Check if a LZMA returned code is not a bad code
        static void assert_lzma_ret(lzma_ret code);

        //Write out current block
        std::size_t compress_buffer(std::size_t in_size) override;

        void init_compressor() override;
        
    public:
        BlockCompressorLZMA() = default;
};

#endif