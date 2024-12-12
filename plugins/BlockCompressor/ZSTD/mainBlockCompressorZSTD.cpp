#include <iostream>
#include <chrono>
#include <string>

#include <BlockCompressorZSTD.h>

int main(int argc, char ** argv)
{
    if(argc != 7)
    {
        std::cout << "Usage: ./mainBlockCompressorZSTD <matrix> <config> <hash_info> <partition> <header> <prefix>" << std::endl;
        return 2;
    }

    std::string in_path = argv[1];
    std::string config_path = argv[2];
    std::string hash_info_path = argv[3];
    unsigned partition = (unsigned)std::stoi(argv[4]);
    unsigned short header_size = (unsigned short)std::stoi(argv[5]);
    std::string prefix = argv[6];

    BlockCompressorZSTD bc;    
    BlockCompressorZSTD::compress_cmbf(bc, in_path, prefix, config_path, hash_info_path, partition, header_size);
}