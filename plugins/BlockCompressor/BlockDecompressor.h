#ifndef BLOCKDECOMPRESSOR_H
#define BLOCKDECOMPRESSOR_H

#include <cassert>
#include <ConfigurationLiterate.h>
#include <fstream>
#include <lzma.h>
#include <vector>
#include <algorithm>
#include <sdsl/bit_vectors.hpp>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressor
{
    private:
        //Properties
        ConfigurationLiterate config; //Configuration class { preset_level, bit_vectors_per_block, nb_samples }

        //Buffers and block variables
        std::vector<std::uint8_t> in_buffer; //Buffer to store block encoded data
        std::vector<std::uint8_t> out_buffer; //Buffer to store block decoded data

        bool read_once = false; //Flag is set as soon as a block has been decoded
        std::size_t decoded_block_index = 0; //Store the last decoded block index
        std::size_t decoded_block_size = 0; //Store the last decoded block size
        std::size_t BLOCK_DECODED_SIZE = 0; //Expected size of a decoded block (in bytes)
        std::size_t bit_vector_size = 0; //Size of a decoded bit_vector (in bytes)
        std::uint64_t minimum_hash;
        
        //IO variables
        std::ifstream matrix; //Input file stream of compressed matrix
        std::ifstream ef_in; //Input file stream of serialized Elias-Fano

        //EF data
        sdsl::sd_vector<> ef; //Elias-Fano object used to store final blocks starting location
        sdsl::sd_vector<>::select_1_type ef_pos; //Select support for Elias-Fano
        std::size_t ef_size;

        //LZMA options        
        lzma_filter filters[2]; //Filters used by XZ
        lzma_options_lzma opt_lzma; //Options used in XZ filters

        static void assert_lzma_ret(lzma_ret code);

        //Decode i-th block if not currently loaded in memory
        void decode_block(std::size_t i);
    public:
        BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path);

        BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path);

        //Decodes the block containing the corresponding hash value and that return the corresponding bit vector 
        const std::uint8_t* get_bit_vector_from_hash(std::uint64_t hash);

        //Retrieve original matrix in a specified file
        void decompress_all(const std::string& out_path);

        //Mainly for testing purpose: 
        //Ask next query to decompress block even if it was already decompressed
        //This method doesn't free memory at all
        void unload();

        //Get size of bit vectors for loaded matrix
        std::uint64_t get_bit_vector_size() const;
};

#endif