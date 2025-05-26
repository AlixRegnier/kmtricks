#include <iostream>
#include <chrono>
#include <string>

#include <BlockCompressorZSTD.h>

int main(int argc, char ** argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: mainBlockCompressorZSTD <matrix> <config> <header> <prefix>\n\n";
        return 1;
    }

    std::string in_path = argv[1];
    std::string config_path = argv[2];
    unsigned short header_size = (unsigned short)std::stoi(argv[3]);
    std::string prefix = argv[4];

    BlockCompressorZSTD bc;    
    BlockCompressorZSTD::compress_cmbf(bc, in_path, prefix, config_path, header_size);
}