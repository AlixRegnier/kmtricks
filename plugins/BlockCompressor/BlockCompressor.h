#ifndef BLOCKCOMPRESSOR_H
#define BLOCKCOMPRESSOR_H

#define KMTRICKS_PUBLIC
#include <cassert>
#include <kmtricks/plugin.hpp>
#include <kmtricks/utils.hpp>
#include <kmtricks/hash.hpp>

//XZ utils main header
#include <lzma.h>


//Configuration parser
#include <ConfigurationLiterate.h>

#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

//Elias-Fano encoding
#include <sdsl/bit_vectors.hpp>

using count_type = typename km::selectC<DMAX_C>::type;

class BlockCompressor : public km::IMergePlugin
{
    private:
        //Properties
        ConfigurationLiterate config; //Configuration class { preset_level, bit_vectors_per_block, nb_samples }

        //Members to catch missing hashes
        std::uint64_t minimum_hash;
        std::uint64_t maximum_hash;
        std::uint64_t previous_hash;

        //Buffers and IO variables
        std::uint64_t current_size = 0;
        std::vector<std::uint8_t> m_buffer;
        std::vector<std::uint8_t> in_buffer;
        std::vector<std::uint8_t> out_buffer;
        std::size_t in_buffer_current_size = 0;
        std::size_t bit_vectors_read = 0;
        std::ofstream m_out;

        //EF data
        std::ofstream ef_out;
        std::vector<std::uint64_t> ef_pos;

        //LZMA options
        lzma_options_lzma opt_lzma;
        lzma_filter filters[2];
    
        //Check if a LZMA returned code is not a bad code
        static void assert_lzma_ret(lzma_ret code);

        //Append bit_vector to block buffer
        //By default, it uses the internal bit_vector
        //Otherwise, use given bit_vector: need to have a size greater or equal to configured bit_vector size
        void add_buffer_to_block(const std::uint8_t * bit_vector = nullptr);

        //Write out <n> bit_vectors filled with zeroes
        void fill_zero_buffers(std::uint64_t n);

        //Write out current block
        void write_block();

        //Write out Elias-Fano representation
        void write_elias_fano();
        
    public:
        BlockCompressor();
        ~BlockCompressor();

        
        //Add hash to compressed matrix (missing hash are filled with zeroes in matrix)
        //<counts> is not binarized
        bool process_hash(std::uint64_t hash, std::vector<count_type>& counts) override;

        //Add hash to compressed matrix (missing hash are filled with zeroes in matrix)
        //<bit_vector> is already binarized
        void process_binarized_bit_vector(std::uint64_t hash, const std::uint8_t* bit_vector);

        //Compress hash:pa:bin kmtricks matrix
        static void compress_pa_hash(const std::string& in_path, const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned partition, unsigned short skip_header = 37);

        //Compress hash:bf:bin kmtricks matrix
        static void compress_cmbf(const std::string& in_path, const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned partition, unsigned short skip_header = 37);

        //As kmtricks plugin only: 'config_path' is the string passed to --plugin-config, path to the config file
        void configure(const std::string& config_path) override;

        //Configure instance when not used as kmtricks plugin. 
        //Should not be directly used. In that case, this method is called by compress_XXX
        void no_plugin_configure(const std::string& out_prefix, const std::string& config_path, const std::string& hash_info_path, unsigned partition);
};

#endif