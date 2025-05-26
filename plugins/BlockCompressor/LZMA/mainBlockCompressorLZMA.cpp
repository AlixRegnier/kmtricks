#include <iostream>
#include <chrono>
#include <string>

#include <BlockCompressorLZMA.h>

int main(int argc, char ** argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: mainBlockCompressorLZMA <matrix> <config> <header> <prefix>\n\n";
        return 1;
    }

    std::string in_path = argv[1];
    std::string config_path = argv[2];
    std::string prefix = argv[3];
    unsigned short header_size = (unsigned short)std::stoi(argv[4]);

    BlockCompressorLZMA bc;    
    BlockCompressorLZMA::compress_cmbf(bc, in_path, prefix, config_path, header_size);
}