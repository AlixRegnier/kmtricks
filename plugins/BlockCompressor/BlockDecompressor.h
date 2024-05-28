#ifndef BLOCKDECOMPRESSOR_H
#define BLOCKDECOMPRESSOR_H

#include <cassert>
#include <ConfigurationLiterate.h>
#include <fstream>
#include <lzma.h>
#include <vector>

class BlockDecompressor
{
    private:
        lzma_filter filters[2]; //Filters used by XZ
        lzma_options_lzma opt_lzma; //Options used in XZ filters

        std::vector<std::uint8_t> in_buffer; //Buffer to store block encoded data
        std::vector<std::uint8_t> out_buffer; //Buffer to store block decoded data

        //TODO Add a proper and serializable EF implementation instead of a vector of integers
        std::vector<std::uint64_t> ef_pos; 

        bool read_once = false; //Flag is set as soon as a block has been decoded
        std::size_t decoded_block_index = 0; //Store the last decoded block index
        std::size_t decoded_block_size = 0;
        std::size_t BLOCK_DECODED_SIZE = 0; //Expected size of a decoded block (in bytes)
        std::size_t bit_vector_size = 0; //Size of a decoded bit_vector (in bytes)
        ConfigurationLiterate config; //Configuration class { preset_level, bit_vectors_per_block, nb_samples }
        std::ifstream matrix; //Input file stream of compressed matrix
        std::ifstream ef_in; //Input file stream of serialized Elias-Fano

        static void assert_lzma_ret(lzma_ret code);
        void decode_block(std::size_t i);
    public:
        BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path);

        BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path);

        const std::uint8_t* get_bit_vector_from_hash(std::uint64_t hash);

        void decompress_all(const std::string& out_path);
        void unload();
        std::uint64_t get_bit_vector_size() const;
};

#endif