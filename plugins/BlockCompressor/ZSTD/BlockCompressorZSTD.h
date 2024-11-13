#ifndef BLOCKCOMPRESSORZSTD_H
#define BLOCKCOMPRESSORZSTD_H

#include <BlockCompressor.h>

//XZ utils main header
#include <zstd.h>

class BlockCompressorZSTD : public BlockCompressor
{
    private:
        //ZSTD options
        ZSTD_CCtx* context;

        //Write out current block
        std::size_t compress_buffer(std::size_t in_size) override;

        void init_compressor() override;
        
    public:
        BlockCompressorZSTD() = default;
        ~BlockCompressorZSTD();
};

#endif