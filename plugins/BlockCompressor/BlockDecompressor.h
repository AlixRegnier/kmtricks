#ifndef BLOCKDECOMPRESSOR_H
#define BLOCKDECOMPRESSOR_H

#include <cassert>
#include <iostream>
#include <fstream>
#include "ConfigurationLiterate.h"
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
        std::size_t line_size; //Size of a decoded line (in bytes)
        std::uint64_t block_decoded_size; //Expected size of a decoded block (in bytes)
        ConfigurationLiterate config; //Configuration class { preset_level, lines_per_block, nb_samples }
        std::ifstream matrix; //Input file stream of compressed matrix
        std::ifstream ef_in; //Input file stream of serialized Elias-Fano

        void assert_lzma_ret(lzma_ret);
        std::size_t decode_block(std::size_t);
    public:
        BlockDecompressor(const std::string&, const std::string&, const std::string&);

        BlockDecompressor(const ConfigurationLiterate&, const std::string&, const std::string&);

        const std::uint8_t* get_bit_vector_from_hash(std::uint64_t);

        void decompress_all(const std::string& out_path);

        std::uint64_t get_line_size() const;
};

#endif