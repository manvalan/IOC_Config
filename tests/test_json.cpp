#include "ioc_config/oop_parser.h"
#include <iostream>
#include <fstream>

using namespace ioc_config;

void testJsonStringLoading() {
    std::cout << "\n=== Test: JSON String Loading ===\n";
    
    std::string jsonStr = R"({
        "object": {
            "id": "17030",
            "name": "Asteroid 17030"
        },
        "time": {
            "start_date": "2025-11-25",
            "end_date": "2025-12-02"
        },
        "search": {
            "max_magnitude": 16.0,
            "min_elevation_deg": 20.0
        }
    })";
    
    OopParser parser;
    if (parser.loadFromJsonString(jsonStr)) {
        std::cout << "✓ Successfully loaded JSON string\n";
        std::cout << "  Sections: " << parser.getSectionCount() << "\n";
        
        // Verify sections
        auto sections = parser.getAllSections();
        for (const auto& section : sections) {
            std::cout << "  - " << section.name << ": " << section.parameters.size() << " params\n";
        }
    } else {
        std::cout << "✗ Failed to load JSON string\n";
        std::cout << "  Error: " << parser.getLastError() << "\n";
    }
}

void testJsonStringOutput() {
    std::cout << "\n=== Test: JSON String Output ===\n";
    
    OopParser parser;
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'Asteroid 17030'");
    parser.setParameter("search", ".max_magnitude", "16.0");
    
    std::string jsonOutput = parser.toJsonString(2);
    std::cout << "✓ Generated JSON output:\n";
    std::cout << jsonOutput << "\n";
}

void testJsonObjectConversion() {
    std::cout << "\n=== Test: JSON Object Conversion ===\n";
    
    // Create JSON object
    nlohmann::json config;
    config["object"]["id"] = "17030";
    config["object"]["name"] = "Test Asteroid";
    config["time"]["start_date"] = "2025-11-25";
    config["time"]["end_date"] = "2025-12-02";
    config["search"]["max_magnitude"] = 16.0;
    
    // Load from JSON object
    OopParser parser;
    if (parser.fromJson(config)) {
        std::cout << "✓ Successfully loaded from JSON object\n";
        
        // Convert back to JSON
        auto outputJson = parser.toJson();
        std::cout << "✓ Converted back to JSON object\n";
        std::cout << "  Output: " << outputJson.dump(2) << "\n";
    } else {
        std::cout << "✗ Failed to load from JSON object\n";
    }
}

void testBidirectionalConversion() {
    std::cout << "\n=== Test: Bidirectional Conversion ===\n";
    
    // Create a config from JSON string
    std::string originalJson = R"({
        "object": {"id": "17030", "name": "Test"},
        "time": {"start_date": "2025-11-25", "end_date": "2025-12-02"},
        "search": {"max_magnitude": 16.0}
    })";
    
    OopParser parser;
    parser.loadFromJsonString(originalJson);
    
    // Save to JSON file
    if (parser.saveToJson("test_config.json")) {
        std::cout << "✓ Saved configuration to JSON file\n";
        
        // Load it back
        OopParser parser2;
        if (parser2.loadFromJson("test_config.json")) {
            std::cout << "✓ Loaded configuration from JSON file\n";
            
            // Get as string again
            std::string roundtripJson = parser2.toJsonString();
            std::cout << "✓ Roundtrip successful\n";
            std::cout << "  Final JSON:\n" << roundtripJson << "\n";
        }
    }
}

void testValidation() {
    std::cout << "\n=== Test: Configuration Validation ===\n";
    
    OopParser parser;
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'Test'");
    parser.setParameter("time", ".start_date", "'2025-11-25'");
    parser.setParameter("time", ".end_date", "'2025-12-02'");
    parser.setParameter("search", ".max_magnitude", "16.0");
    
    std::vector<std::string> errors;
    if (parser.validate(errors)) {
        std::cout << "✓ Configuration is valid\n";
    } else {
        std::cout << "✗ Configuration validation failed:\n";
        for (const auto& err : errors) {
            std::cout << "  - " << err << "\n";
        }
    }
}

int main() {
    std::cout << "IOC_Config - JSON Functionality Tests\n";
    std::cout << "=====================================\n";
    
    try {
        testJsonStringLoading();
        testJsonStringOutput();
        testJsonObjectConversion();
        testBidirectionalConversion();
        testValidation();
        
        std::cout << "\n=====================================\n";
        std::cout << "All JSON tests completed! ✓\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << "\n";
        return 1;
    }
}
