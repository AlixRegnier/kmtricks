#include <BlockDecompressor.h>

int main(int argc, char ** argv)
{
    if(argc != 5)
    {
        std::cout << "Usage: ./BlockDecompressor <config_file> <matrix> <ef_path> <output>\n\n";
        exit(2);
    }

    //Initialize decompressor and decompressor each blocks to <output>
    BlockDecompressor(argv[1], argv[2], argv[3]).decompress_all(argv[4]);
}