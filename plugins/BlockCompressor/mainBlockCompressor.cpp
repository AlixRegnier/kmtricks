#include <iostream>
#include <chrono>
#include <string>

#include <BlockCompressor.h>

int main(int argc, char ** argv)
{
    if(argc != 6)
    {
        std::cout << "Usage: compress <matrix> <config> <hash_info> <prefix> <header>" << std::endl;
        return 2;
    }

    std::string in_path = argv[1];
    std::string config_path = argv[2];
    std::string hash_info_path = argv[3];
    std::string prefix = argv[4];
    unsigned short header_size = (unsigned short)std::stoi(argv[5]);
    
    BlockCompressor::compress_cmbf(in_path, prefix, config_path, hash_info_path, header_size);
}