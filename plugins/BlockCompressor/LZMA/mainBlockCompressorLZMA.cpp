#include <iostream>
#include <chrono>
#include <string>

#include <BlockCompressorLZMA.h>

int main(int argc, char ** argv)
{
    if(argc != 7)
    {
        std::cout << "Usage: ./mainBlockCompressorLZMA <matrix> <config> <hash_info> <partition> <header> <prefix>" << std::endl;
        return 2;
    }

    std::string in_path = argv[1];
    std::string config_path = argv[2];
    std::string hash_info_path = argv[3];
    unsigned partition = (unsigned)std::stoi(argv[4]);
    std::string prefix = argv[5];
    unsigned short header_size = (unsigned short)std::stoi(argv[6]);

    BlockCompressorLZMA bc;    
    BlockCompressorLZMA::compress_cmbf(bc, in_path, prefix, config_path, hash_info_path, partition, header_size);
}