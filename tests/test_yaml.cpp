/**
 * @file test_yaml.cpp
 * @brief IOC_Config - YAML Support Test Suite
 * 
 * Tests YAML I/O functionality when yaml-cpp support is available
 * 
 * @author Michele Bigi (mikbigi@gmail.com)
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <fstream>
#include <cstdio>

using namespace ioc_config;

int main() {
    int passed = 0, failed = 0;
    
    std::cout << "IOC_Config - YAML Support Test Suite\n";
    std::cout << "====================================\n\n";

    #ifdef IOC_CONFIG_YAML_SUPPORT
    
    std::cout << "✓ YAML support is enabled\n\n";
    
    // Test 1: Create configuration and save to YAML file
    std::cout << "=== Test 1: Save to YAML File ===\n";
    {
        OopParser parser;
        parser.setParameter("object", ".id", "'asteroid_001'");
        parser.setParameter("object", ".name", "'Test Asteroid'");
        parser.setParameter("time", ".start", "'2024-01-01'");
        
        std::string yaml_file = "/tmp/test_config.yaml";
        if (parser.saveToYaml(yaml_file)) {
            std::cout << "✓ PASS: Configuration saved to YAML file\n";
            passed++;
            
            // Verify file exists
            std::ifstream file(yaml_file);
            if (file.good()) {
                std::cout << "✓ PASS: YAML file exists and is readable\n";
                passed++;
            } else {
                std::cout << "✗ FAIL: YAML file not readable\n";
                failed++;
            }
        } else {
            std::cout << "✗ FAIL: Could not save YAML file\n";
            failed++;
        }
    }
    std::cout << "\n";
    
    // Test 2: Load from YAML file
    std::cout << "=== Test 2: Load from YAML File ===\n";
    {
        OopParser parser;
        std::string yaml_file = "/tmp/test_config.yaml";
        
        if (parser.loadFromYaml(yaml_file)) {
            std::cout << "✓ PASS: Configuration loaded from YAML file\n";
            passed++;
            
            if (parser.getSectionCount() > 0) {
                std::cout << "✓ PASS: Sections loaded (" << parser.getSectionCount() << " sections)\n";
                passed++;
            } else {
                std::cout << "✗ FAIL: No sections loaded\n";
                failed++;
            }
        } else {
            std::cout << "✗ FAIL: Could not load YAML file\n";
            failed++;
        }
    }
    std::cout << "\n";
    
    // Test 3: Save to YAML string
    std::cout << "=== Test 3: Save to YAML String ===\n";
    {
        OopParser parser;
        parser.setParameter("object", ".id", "'test_id'");
        parser.setParameter("search", ".max_magnitude", "18.5");
        
        std::string yaml_string = parser.saveToYamlString();
        if (!yaml_string.empty()) {
            std::cout << "✓ PASS: Configuration converted to YAML string\n";
            std::cout << "  String length: " << yaml_string.length() << " bytes\n";
            passed++;
        } else {
            std::cout << "✗ FAIL: YAML string is empty\n";
            failed++;
        }
    }
    std::cout << "\n";
    
    // Test 4: Load from YAML string
    std::cout << "=== Test 4: Load from YAML String ===\n";
    {
        std::string yaml_input = R"(
object:
  .id: "'loaded_from_string'"
  .name: "'String Config'"
time:
  .start: "'2024-06-15'"
search:
  .max_magnitude: "16.0"
)";
        
        OopParser parser;
        if (parser.loadFromYamlString(yaml_input)) {
            std::cout << "✓ PASS: Configuration loaded from YAML string\n";
            passed++;
            
            if (parser.getSectionCount() == 3) {
                std::cout << "✓ PASS: All 3 sections loaded correctly\n";
                passed++;
            } else {
                std::cout << "✗ FAIL: Expected 3 sections, got " << parser.getSectionCount() << "\n";
                failed++;
            }
        } else {
            std::cout << "✗ FAIL: Could not load from YAML string\n";
            failed++;
        }
    }
    std::cout << "\n";
    
    // Test 5: Round-trip (save then load)
    std::cout << "=== Test 5: Round-trip (Save then Load) ===\n";
    {
        OopParser parser1;
        parser1.setParameter("object", ".id", "'round_trip_test'");
        parser1.setParameter("time", ".start", "'2024-12-25'");
        parser1.setParameter("search", ".min_duration_sec", "5.0");
        
        size_t original_sections = parser1.getSectionCount();
        std::string yaml_file = "/tmp/roundtrip_test.yaml";
        
        if (parser1.saveToYaml(yaml_file)) {
            OopParser parser2;
            if (parser2.loadFromYaml(yaml_file)) {
                if (parser2.getSectionCount() == original_sections) {
                    std::cout << "✓ PASS: Round-trip successful (sections match)\n";
                    passed++;
                } else {
                    std::cout << "✗ FAIL: Section count mismatch after round-trip\n";
                    failed++;
                }
            } else {
                std::cout << "✗ FAIL: Could not load after save\n";
                failed++;
            }
        } else {
            std::cout << "✗ FAIL: Could not save for round-trip test\n";
            failed++;
        }
    }
    std::cout << "\n";
    
    #else
    
    std::cout << "⚠ YAML support is NOT enabled\n";
    std::cout << "To enable YAML support:\n";
    std::cout << "  macOS: brew install yaml-cpp\n";
    std::cout << "  Linux: apt-get install libyaml-cpp-dev\n";
    std::cout << "  Then rebuild the project\n\n";
    
    // Test that stubs exist and return false gracefully
    std::cout << "=== Test 1: Stubs return false gracefully ===\n";
    {
        OopParser parser;
        parser.setParameter("object", ".id", "'test'");
        
        // These should return false gracefully when YAML support is disabled
        bool save_result = parser.saveToYaml("/tmp/test.yaml");
        bool load_result = parser.loadFromYaml("/tmp/test.yaml");
        bool load_str_result = parser.loadFromYamlString("object:\n  .id: 'test'");
        std::string save_str_result = parser.saveToYamlString();
        
        if (!save_result && !load_result && !load_str_result && save_str_result.empty()) {
            std::cout << "✓ PASS: All YAML stubs return gracefully when disabled\n";
            passed++;
        } else {
            std::cout << "✗ FAIL: Stubs did not behave as expected\n";
            failed++;
        }
    }
    
    #endif
    
    // Print summary
    std::cout << "====================================\n";
    std::cout << "Test Results\n";
    std::cout << "====================================\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Total:  " << (passed + failed) << "\n";
    
    if (failed == 0) {
        std::cout << "\n✓ All tests passed!\n";
    } else {
        std::cout << "\n✗ Some tests failed\n";
    }
    
    return (failed == 0) ? 0 : 1;
}
