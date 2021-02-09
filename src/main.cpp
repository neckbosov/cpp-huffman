#include "huffman.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 6) {
        std::cerr << "Incorrect number of arguments, must be 5" << std::endl;
        exit(1);
    }
    bool encode;
    if (strcmp(argv[1], "-c") == 0) {
        encode = true;
    }
    
    else if (strcmp(argv[1], "-u") == 0) {
        encode = false;
    }
    else {
        std::cerr << "First argument must be -c or -u" << std::endl;
        exit(1);
    }
    std::string input_file(argv[3]), output_file(argv[5]);
    if (!strcmp(argv[2], "-f") && !strcmp(argv[2], "--file")) {
        std::cerr << "Second argument must be -f or --file" << std::endl;
        exit(1);
    }
    if (!strcmp(argv[4], "-o") && !strcmp(argv[4], "--output")) {
        std::cerr << "Fourth argument must be -o or --output" << std::endl;
        exit(1);
    }
    HuffmanArchiever archiever(input_file, output_file);
    if (encode) {
        archiever.encode();
    }
    else {
        archiever.decode();
    }
    return 0;
}