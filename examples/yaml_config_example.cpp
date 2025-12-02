/**
 * @file yaml_config_example.cpp
 * @brief IOC_Config - YAML Configuration Example
 * 
 * Demonstrates YAML file I/O operations with IOC_Config library
 * 
 * @author Michele Bigi (mikbigi@gmail.com)
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    std::cout << "\n=================================================\n";
    std::cout << "IOC_Config - YAML Configuration Example\n";
    std::cout << "=================================================\n\n";

    // Create a parser and populate with configuration
    OopParser parser;
    
    // Add sections and parameters using setParameter
    // Note: setParameter will create sections if they don't exist
    parser.setParameter("object", ".id", "'2022 PN'");
    parser.setParameter("object", ".name", "'Asteroid 2022 PN'");
    
    parser.setParameter("time", ".start", "'2024-01-01T00:00:00'");
    parser.setParameter("time", ".end", "'2024-12-31T23:59:59'");
    
    parser.setParameter("search", ".max_magnitude", "15.5");
    parser.setParameter("search", ".min_duration_sec", "2.0");
    
    std::cout << "=== Configuration Created ===\n";
    std::cout << "Sections: " << parser.getSectionCount() << "\n\n";
    
    // Save to YAML file
    #ifdef IOC_CONFIG_YAML_SUPPORT
    std::cout << "=== Saving to YAML File ===\n";
    std::string yaml_file = "/tmp/ioc_config_example.yaml";
    if (parser.saveToYaml(yaml_file)) {
        std::cout << "✓ Configuration saved to: " << yaml_file << "\n\n";
    } else {
        std::cout << "✗ Failed to save YAML file\n\n";
    }
    
    // Convert to YAML string
    std::cout << "=== YAML String Representation ===\n";
    std::string yaml_string = parser.saveToYamlString();
    if (!yaml_string.empty()) {
        std::cout << yaml_string << "\n";
    } else {
        std::cout << "(YAML support not available)\n";
    }
    std::cout << "\n";
    
    // Load from YAML file
    std::cout << "=== Loading from YAML File ===\n";
    OopParser parser2;
    if (parser2.loadFromYaml(yaml_file)) {
        std::cout << "✓ Configuration loaded from: " << yaml_file << "\n";
        std::cout << "  Sections loaded: " << parser2.getSectionCount() << "\n";
        std::cout << "  Object ID: " << parser2.getParameter("object", ".id") << "\n";
        std::cout << "  Time Start: " << parser2.getParameter("time", ".start") << "\n\n";
    } else {
        std::cout << "✗ Failed to load YAML file\n\n";
    }
    
    // Load from YAML string
    std::cout << "=== Loading from YAML String ===\n";
    std::string yaml_input = R"(
object:
  .id: "'Loaded from String'"
  .name: "'Test Object'"
time:
  .start: "'2024-06-15T12:00:00'"
)";
    
    OopParser parser3;
    if (parser3.loadFromYamlString(yaml_input)) {
        std::cout << "✓ Configuration loaded from YAML string\n";
        std::cout << "  Object ID: " << parser3.getParameter("object", ".id") << "\n";
        std::cout << "  Time Start: " << parser3.getParameter("time", ".start") << "\n\n";
    } else {
        std::cout << "✗ Failed to load YAML from string\n";
        std::cout << "(YAML support not enabled - this is expected if yaml-cpp is not installed)\n\n";
    }
    
    #else
    std::cout << "=== YAML Support ===\n";
    std::cout << "YAML support is not enabled (yaml-cpp not found during build)\n";
    std::cout << "To enable YAML support:\n";
    std::cout << "  macOS: brew install yaml-cpp\n";
    std::cout << "  Linux: apt-get install libyaml-cpp-dev\n";
    std::cout << "  Then rebuild the project\n\n";
    #endif
    
    std::cout << "=================================================\n";
    std::cout << "Example completed successfully!\n";
    std::cout << "=================================================\n\n";
    
    return 0;
}
