#ifndef BLOCKDECOMPRESSOR_H
#define BLOCKDECOMPRESSOR_H

#include <cassert>
#include <ConfigurationLiterate.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sdsl/bit_vectors.hpp>

//Class allowing decompression on fly / total decompression of compressed matrices
//Instances are used for querying matrix lines or as decompressor 
class BlockDecompressor
{
    protected:
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
        //std::uint64_t minimum_hash;
        
        //IO variables
        std::ifstream matrix; //Input file stream of compressed matrix
        std::ifstream ef_in; //Input file stream of serialized Elias-Fano
        unsigned short header_size;

        //EF data
        sdsl::sd_vector<> ef; //Elias-Fano object used to store final blocks starting location
        sdsl::sd_vector<>::select_1_type ef_pos; //Select support for Elias-Fano
        std::uint64_t ef_size;

        //Decode i-th block if not currently loaded in memory
        void decode_block(std::size_t i);

        //Decompress "in_buffer" --> "out_buffer", must return the number of written bytes in out_buffer
        virtual std::size_t decompress_buffer(std::size_t in_size) = 0;
    public:
        BlockDecompressor(const std::string& config_path, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);

        BlockDecompressor(const ConfigurationLiterate& config, const std::string& matrix_path, const std::string& ef_path, unsigned short header_size = 49);

        //Decodes the block containing the corresponding hash value and that return the corresponding bit vector address
        //Returns nullptr if hash is out of range
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