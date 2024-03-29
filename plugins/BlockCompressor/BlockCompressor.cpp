#define KMTRICKS_PUBLIC
#include <cassert>
#include <kmtricks/plugin.hpp>
#include <kmtricks/utils.hpp>
#include <kmtricks/hash.hpp>

//XZ utils main header
#include <lzma.h>

#include <fstream>
#include <iostream>

//Elias-Fano encoding
//#include <elias_fano.h>

using count_type = typename km::selectC<DMAX_C>::type;

class BlockCompressor : public km::IMergePlugin
{
    private:
        
        static int instances;
        std::ofstream m_out;
        std::vector<std::uint8_t> m_buffer;
        std::size_t m_nb_samples;
        std::size_t out_buffer_size;
        std::size_t lines_read;
        std::uint64_t minimum_hash;
        std::uint64_t current_hash;
        std::uint64_t lines_per_block;
        std::uint8_t preset_level;

        std::vector<std::uint8_t> in_buffer;
        std::size_t in_buffer_current_size;
        std::vector<std::uint8_t> out_buffer;
        std::uint64_t current_size;
        std::vector<std::uint64_t> ef_pos;
    

        //const std::lzma_filter * FILTERS = { LZAMA1};
        void assert_lzma_ret(lzma_ret code)
        {
            switch(code)
            {
                case LZMA_OK:
                case LZMA_STREAM_END:
                    return;
                case LZMA_BUF_ERROR:
                    throw std::runtime_error("Not enough memory space allocated for buffer");
                case LZMA_MEM_ERROR:
                    throw std::runtime_error("Not enough memory space on machine");
                default:
                    throw std::runtime_error("LZMA return code not handled");
            }
        }

        //May be used later
        std::uint8_t padding4(std::size_t size)
        {
            /* Equivalent code with if-else statement
            *
            * x = size % 4;
            * 
            * // x=0 => 0
            * // x=1 => 3
            * // x=2 => 2
            * // x=3 => 1
            * 
            * if(size % 4 == 0)
            *     return size + 0;
            * else
            *     return size + 4 - (size % 4);
            */

            std::size_t a = (size >> 1) & 0x1;
            std::size_t b = size & 0x1;

            return size + (((a ^ b) << 1) | b);
        }

        void write_buffer()
        {
            // Note that LZMA_CHECK_NONE won't check encoding integrity (by default it is done with CRC32 but will need a bit of memory for each blocks)
            std::size_t out_size;
    
            //lzma_ret code = lzma_raw_buffer_encode(NULL, NULL, in_buffer.data(), in_buffer_current_size, out_buffer.data(), &out_size, out_buffer_size);
            lzma_ret code = lzma_easy_buffer_encode(LZMA_PRESET_DEFAULT, LZMA_CHECK_NONE, NULL, in_buffer.data(), in_buffer_current_size, out_buffer.data(), &out_size, out_buffer_size);
            assert_lzma_ret(code);

            //Elias-Fano 
            current_size += out_size;
            ef_pos.push_back(current_size);

            m_out.write(reinterpret_cast<char*>(out_buffer.data()), out_size);
        }


        std::string& to_lower_case(std::string& s)
        {
            for(size_t i = 0; i < s.size(); ++i)
                if(s[i] >= 'A' && s[i] <= 'Z')
                    s[i] += (char)32;
            return s;
        }

    public:
        BlockCompressor(std::uint64_t lines_per_block = (std::uint64_t)1, std::uint8_t preset_level = (std::uint8_t)6) : lines_per_block(lines_per_block), preset_level(preset_level)
        {
            ++instances;
            std::cout << "CONSTRUCTOR " << instances << "\n";
            std::cout << "dir: " << this->m_output_directory << "\n";
            std::cout << "min hash: " << this->minimum_hash << "\n";
            std::cout << "partition: " << this->m_partition  << "\n";
            std::cout << "END CONSTRUCTOR " << instances << "\n";
            ef_pos.push_back(0);
        }

        ~BlockCompressor()
        {
            /*if(in_buffer_current_size != 0)
                write_buffer();

            //Elias-Fano encoding
            buildEliasFano();

            m_out.close();*/
        }

        void build_elias_fano()
        {
            /*
            constexpr bool build_index_on_zeros = true;
            elias_fano::sequence<build_index_on_zeros> ef;
            ef.encode(ef_pos.begin(), ef_pos.size(), ef_pos.back());   */ 
        }

        void add_buffer_to_block()
        {
            //Fill vector starting from its "current size"
            for(std::size_t i = 0; i < m_buffer.size(); ++i)
                in_buffer[in_buffer_current_size + i] = m_buffer[i];

            ++lines_read;

            // Update variables tracking data to use in in_buffer vector
            in_buffer_current_size = m_buffer.size() * (lines_read % lines_per_block);

            if(lines_read % lines_per_block == 0)
            {
                // Compress block
                // write_buffer();
            }
        }

        // 'hash' is the hash value
        // 'counts' is the abundance vector of the line, not binarized.
        // Note that you may not see all the lines here, as 'process_hash' is not called on empty lines, i.e. hashes not present in any samples. I don't know how you want to handle this, in the non compressed index, I simply write empty bit vectors.
        bool process_hash(std::uint64_t hash, std::vector<count_type>& counts) override
        {
            if(hash != minimum_hash)
            {
                assert(hash > current_hash);
                std::uint64_t diff = hash - current_hash - 1;

                // Add missing hashes
                if(diff >= 1)
                {
                    std::fill(m_buffer.begin(), m_buffer.end(), 0);
                    for(std::uint64_t i = 0; i < diff; ++i)
                        add_buffer_to_block();
                }
            }

            current_hash = hash;
            km::set_bit_vector(m_buffer, counts); // binarize counts into m_buffer
            add_buffer_to_block();
            
            return false; // You manage output yourself, tell kmtricks to not write the line.
        }


        // 's' is the string passed to --plugin-config, e.g. a path to a config file
        void configure(const std::string& s) override
        {
            //Parse configuration file
            //Assignments must be specified this way: 
            //  PROPERTY = VALUE

            std::string property;
            std::size_t value;
            std::ifstream config(s);
            char equalSign;

            while(config)
            {
                std::string line;
                std::getline(config, line);
                std::istringstream iss(line);
            
                if (iss >> property >> equalSign >> value && (equalSign == '=' || equalSign == ':'))
                    set_property(to_lower_case(property), value);
                else if(line != "")
                    throw std::invalid_argument("Invalid format for property, received: '" + line + "'");
            }

            //Check if parameters values are valid
            if(m_nb_samples == 0)
                throw std::invalid_argument("Number of samples needs to be specified with 'nb_samples = VALUE'");

            if(preset_level > 9)
                throw std::invalid_argument("Preset compression level shall be in [0;9]");

            this->current_hash = this->minimum_hash = km::HashWindow("/scratch/aregnier/xz_block_compression/ecoli_index_simkamin/hash.info").get_lower(this->m_partition);

            std::cout << "CONFIG " << instances << "\n";
            std::cout << "min hash: " << minimum_hash << "\npartition: " << m_partition << "\n";
            std::cout << "END CONFIG " << instances << "\n";

            //Configure buffer size according to parameters
            m_buffer.resize((m_nb_samples + 7) / 8);
            std::string part_str = std::to_string(this->m_partition);
            std::string output_path = this->m_output_directory + "/matrices/matrix_" + part_str;
            m_out.open(output_path, std::ios::out | std::ios::binary);
            in_buffer.resize(m_buffer.size() * lines_per_block);

            // Compression is not inplace, so we need to allocate out_buffer once for storing data
            //Get maximum estimated (upper bound) encoded size
            out_buffer_size = lzma_stream_buffer_bound(sizeof(std::uint8_t) * m_buffer.size() * lines_per_block);
            out_buffer.resize(out_buffer_size);    
        }

        void set_property(const std::string& property, std::size_t value)
        {
            if(property == "samples")
            {
                this->m_nb_samples = value;
            }
            else if(property == "linesperblock")
            {
                this->lines_per_block = value;
            }
            else if(property == "preset")
            {
                this->preset_level = value;
            }
            else
            {
                throw std::runtime_error("Unknown property: '" + property + "'");
            }
        }
};

int BlockCompressor::instances = 0;

extern "C" std::string plugin_name() { return "BlockCompressor"; }
extern "C" int use_template() { return 0; }
extern "C" km::IMergePlugin* create0() { return new BlockCompressor(); }
extern "C" void destroy(km::IMergePlugin* p) { delete p; }
