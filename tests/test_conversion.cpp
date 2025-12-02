#include "ioc_config/oop_parser.h"
#include <iostream>
#include <fstream>

using namespace ioc_config;

// Create a simple test OOP file
void createTestOopFile() {
    std::ofstream file("test_input.oop");
    file << "! Test configuration\n";
    file << "object.\n";
    file << "        .id = '17030'\n";
    file << "        .name = 'TestAsteroid'\n";
    file << "\n";
    file << "time.\n";
    file << "        .start_date = '2025-11-25'\n";
    file << "        .end_date = '2025-12-02'\n";
    file << "\n";
    file << "search.\n";
    file << "        .max_magnitude = 16.0\n";
    file.close();
}

int main() {
    std::cout << "Testing OOP <-> JSON Conversion...\n\n";
    
    // Create test file
    createTestOopFile();
    
    // Test OOP to JSON conversion
    std::cout << "Test: OOP -> JSON conversion\n";
    if (convertOopToJson("test_input.oop", "test_output.json")) {
        std::cout << "  Conversion successful!\n";
    } else {
        std::cout << "  Conversion failed!\n";
        return 1;
    }
    
    // Test JSON to OOP conversion
    std::cout << "\nTest: JSON -> OOP conversion\n";
    if (convertJsonToOop("test_output.json", "test_roundtrip.oop")) {
        std::cout << "  Conversion successful!\n";
    } else {
        std::cout << "  Conversion failed!\n";
        return 1;
    }
    
    std::cout << "\nTest: Conversion complete and PASSED ✓\n";
    return 0;
}
