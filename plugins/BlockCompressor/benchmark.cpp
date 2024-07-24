#include <BlockCompressor.h>
#include <BlockDecompressor.h>
#include <algorithm>
#include <filesystem>
#include <random>
#include <chrono>

#include <kmtricks/utils.hpp>
#include <kmtricks/hash.hpp>

void ASSERT(bool value, const std::string& fail_msg)
{
    if(!value)
    {
        throw std::runtime_error(fail_msg);
    }
}

void bit_vector_to_hex(const std::uint8_t * vec, std::size_t size)
{
    for(int i = 0; i < size; ++i)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(vec[i] & 0xFF) << " ";
    
    std::cout << "###\n";
}


void query_bit_vector_from_cmbf(std::ifstream& in_file, std::uint8_t * buffer, std::uint64_t hash, std::size_t length, unsigned short header)
{
    in_file.seekg(hash * length + header);
    in_file.read(reinterpret_cast<char*>(buffer), length);
}

bool are_equal_cmbf(std::uint64_t minimum_hash, std::uint64_t maximum_hash, std::ifstream& in_file, BlockDecompressor& bd, unsigned short header)
{
    std::uint8_t * bit_vector1 = new std::uint8_t[bd.get_bit_vector_size()];
    const std::uint8_t * bit_vector2 = nullptr;
    
    for(std::uint64_t hash = minimum_hash; hash < maximum_hash + 1; ++hash)
    {
        query_bit_vector_from_cmbf(in_file, bit_vector1, hash - minimum_hash, bd.get_bit_vector_size(), header);
        bit_vector2 = bd.get_bit_vector_from_hash(hash);

        ASSERT(bit_vector2 != nullptr, "Hash couldn't be extracted from compressed matrix");

        for(int i = 0; i < bd.get_bit_vector_size(); i++)
            if(bit_vector1[i] != bit_vector2[i])
            {
                delete[] bit_vector1;
                return false;
            }

        std::cout << "\r" << ((double)(hash - minimum_hash)/(maximum_hash-minimum_hash+1)*100.0);
    }

    std::cout << "\r100.0           \n";

    bd.unload();
    delete[] bit_vector1;
    return true;
}

void measure_random_hash_cmbf(std::uint64_t minimum_hash, std::uint64_t maximum_hash, std::ifstream& in_file, BlockDecompressor& bd, unsigned short header)
{
    const std::uint64_t BIT_VECTOR_SIZE = bd.get_bit_vector_size();
    std::uint8_t * bit_vector = new std::uint8_t[BIT_VECTOR_SIZE];

    //Uniform random Mersenne-Twister generator (64bit)
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dis(minimum_hash, maximum_hash);

    int steps = 5;

    std::vector<std::uint64_t> hash_values;
    hash_values.reserve(steps); //Set vector internal size to <steps>

    for(int i = 0; i < steps; ++i)
        hash_values.push_back(dis(gen));

    std::size_t sum1 = 0, sum2 = 0;
    for(auto hash : hash_values)
    {
        bd.unload(); //Force decompressor to decode block for the next query

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        query_bit_vector_from_cmbf(in_file, bit_vector, hash-minimum_hash, BIT_VECTOR_SIZE, header);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum1 += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        begin = std::chrono::steady_clock::now();
        bd.get_bit_vector_from_hash(hash);
        end = std::chrono::steady_clock::now();
        sum2 += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }

    std::cout << "Average time for reading a bit_vector in original matrix:   " << ((long double)sum1 / steps) << " ms" << std::endl;
    std::cout << "Average time for reading a bit_vector in compressed matrix: " << ((long double)sum2 / steps) << " ms" << std::endl;

    bd.unload();
    delete[] bit_vector;
}

void measure_random_sorted_hash_cmbf(std::uint64_t minimum_hash, std::uint64_t maximum_hash, std::ifstream& in_file, BlockDecompressor& bd, unsigned short header)
{
    const std::uint64_t BIT_VECTOR_SIZE = bd.get_bit_vector_size();
    std::uint8_t * bit_vector = new std::uint8_t[BIT_VECTOR_SIZE];

    //Uniform random Mersenne-Twister generator (64bit)
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dis(minimum_hash, maximum_hash);

    int steps = 10000;

    std::vector<std::uint64_t> hash_values;
    hash_values.reserve(steps); //Set vector internal size to <steps>

    for(int i = 0; i < steps; ++i)
        hash_values.push_back(dis(gen));

    std::sort(hash_values.begin(), hash_values.end());

    std::size_t sum1 = 0, sum2 = 0;
    for(auto hash : hash_values)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        query_bit_vector_from_cmbf(in_file, bit_vector, hash, BIT_VECTOR_SIZE, header);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum1 += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        begin = std::chrono::steady_clock::now();
        bd.get_bit_vector_from_hash(hash);
        end = std::chrono::steady_clock::now();
        sum2 += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }

    std::cout << "Average time for reading a bit_vector in original matrix:   " << ((long double)sum1 / steps) << " ms" << std::endl;
    std::cout << "Average time for reading a bit_vector in compressed matrix: " << ((long double)sum2 / steps) << " ms" << std::endl;
    
    bd.unload();
    delete[] bit_vector;
}

void measure_compression_time_cmbf(const std::string& matrix_path, const std::string& config_path, const std::string& hash_info_path, unsigned partition, unsigned short header)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    BlockCompressor::compress_cmbf(matrix_path, "test_compression", config_path, hash_info_path, partition, header);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Partition compression time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " s" << std::endl;
}

void measure_full_read_time_cmbf(std::uint64_t minimum_hash, std::uint64_t maximum_hash, std::ifstream& in_file, BlockDecompressor& bd, unsigned short header)
{
    const std::uint64_t BIT_VECTOR_SIZE = bd.get_bit_vector_size();
    std::uint8_t * bit_vector = new std::uint8_t[BIT_VECTOR_SIZE];

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for(std::uint64_t hash = minimum_hash; hash < maximum_hash + 1; ++hash)
    {
        query_bit_vector_from_cmbf(in_file, bit_vector, hash - minimum_hash, BIT_VECTOR_SIZE, header);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Matrix full read time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " s" << std::endl;

    begin = std::chrono::steady_clock::now();
    for(std::uint64_t hash = minimum_hash; hash < maximum_hash + 1; ++hash)
    {
        bd.get_bit_vector_from_hash(hash);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Compressed matrix full read time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " s" << std::endl;

    bd.unload();
    delete[] bit_vector;
}

void measure_decompression_time_cmbf(BlockDecompressor& bd)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    bd.decompress_all("test");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Partition decompression time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " s" << std::endl;
    bd.unload();
}

int main(int argc, char ** argv)
{
    if(argc != 8)
    {
        std::cout << "Usage: ./benchmark <matrix> <hash_info_path> <partition> <config> <compressed_matrix> <ef> <header>\n\n";
        exit(2);
    }

    std::string matrix_path = argv[1];
    std::string hash_info_path = argv[2];

    //Parse partition number
    int partition = std::stoi(argv[3]);
    
    std::string config_path = argv[4];
    std::string matrix_compressed_path = argv[5];
    std::string ef_path = argv[6];

    std::filesystem::path path = matrix_path;
    std::ifstream matrix;
    std::ifstream matrix_compressed;

    unsigned short header = (unsigned short)std::stoi(argv[7]);

    //Open matrix
    matrix.open(argv[1], std::ifstream::binary);

    //Get minimum and maximum hash value
    std::uint64_t minimum_hash = km::HashWindow(hash_info_path).get_lower(partition);
    std::uint64_t maximum_hash = km::HashWindow(hash_info_path).get_upper(partition);

    BlockDecompressor bd(config_path, matrix_compressed_path, ef_path);
    
    if(path.extension() == ".pa_hash")
        std::cout << "Not handled yet (need to code dichotomy to search fast same hash value)" << std::endl;
    else if(path.extension() == ".cmbf")
    {
        //All consecutives hash values
        std::cout << "###Equality test###" << std::endl;
        ASSERT(are_equal_cmbf(minimum_hash, maximum_hash, matrix, bd, header), "Original matrix and compressed matrix are not equivalent !");
        std::cout << "Both matrix returned same results for each hash" << std::endl;

        std::cout << "###Random hash###" << std::endl;
        measure_random_hash_cmbf(minimum_hash, maximum_hash, matrix, bd, header);

        std::cout << "###Sorted hash###" << std::endl;
        measure_random_sorted_hash_cmbf(minimum_hash, maximum_hash, matrix, bd, header);
        
        std::cout << "###Full read###" << std::endl;
        measure_full_read_time_cmbf(minimum_hash, maximum_hash, matrix, bd, header);

        std::cout << "###Compression###" << std::endl;
        measure_compression_time_cmbf(matrix_path, config_path, hash_info_path, partition, header);

        std::cout << "###Decompression###" << std::endl;
        measure_decompression_time_cmbf(bd);

    }
    else
        std::cout << "Extension not recognized need { .pa_hash , .cmbf }" << std::endl;

}
