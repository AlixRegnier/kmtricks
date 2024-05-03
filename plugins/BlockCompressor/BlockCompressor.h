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

//Elias-Fano encoding
//#include <succinct/elias_fano.hpp>

using count_type = typename km::selectC<DMAX_C>::type;

class BlockCompressor : public km::IMergePlugin
{
    private:

        //Members to catch missing hashes
        std::uint64_t minimum_hash;
        std::uint64_t maximum_hash;
        std::uint64_t previous_hash;

        //Properties
        ConfigurationLiterate config;

        //Buffers and IO variables
        std::vector<std::uint8_t> m_buffer;
        std::vector<std::uint8_t> in_buffer;
        std::vector<std::uint8_t> out_buffer;
        std::size_t in_buffer_current_size = 0;
        std::size_t bit_vectors_read = 0;
        std::ofstream m_out;

        //EF data
        std::ofstream ef_out;
        std::vector<std::uint64_t> ef_pos;
        //succint::elias_fano::elias_fano_builder ef_builder = {0};

        std::uint64_t current_size = 0;

        //LZMA options
        lzma_options_lzma opt_lzma;
        lzma_filter filters[2];
    
        void assert_lzma_ret(lzma_ret code);

        //May be used later
        std::uint8_t padding4(std::size_t);

        void write_buffer();
        void write_elias_fano();
        void add_buffer_to_block();

    public:
        BlockCompressor();
        ~BlockCompressor();

        // 'hash' is the hash value
        // 'counts' is the abundance vector of the bit_vector, not binarized.
        // Note that you may not see all the bit_vectors here, as 'process_hash' is not called on empty bit_vectors, i.e. hashes not present in any samples. I don't know how you want to handle this, in the non compressed index, I simply write empty bit vectors.
        bool process_hash(std::uint64_t, std::vector<count_type>&) override;

        // 'config_path' is the string passed to --plugin-config, path to the config file
        void configure(const std::string& config_path) override;
};

#endif