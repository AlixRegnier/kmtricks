#include <BlockDecompressorZSTD.h>

int main(int argc, char ** argv)
{
    if(argc != 6)
    {
        std::cout << "Usage: ./mainBlockDecompressorZSTD <config_file> <matrix> <ef_path> <header> <output>\n\n";
        return 1;
    }   

    unsigned short header_size = (unsigned short)std::stoi(argv[4]);

    //Initialize decompressor and decompressor each blocks to <output>
    BlockDecompressorZSTD(argv[1], argv[2], argv[3], header_size).decompress_all(argv[5]);
}