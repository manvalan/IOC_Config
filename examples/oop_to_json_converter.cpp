#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.oop> <output.json>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    std::cout << "Converting OOP to JSON...\n";
    std::cout << "Input:   " << input_file << "\n";
    std::cout << "Output:  " << output_file << "\n";

    if (convertOopToJson(input_file, output_file)) {
        std::cout << "Conversion successful!\n";
        return 0;
    } else {
        std::cerr << "Error: Conversion failed!\n";
        return 1;
    }
}
