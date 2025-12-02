#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    std::cout << "IOC_Config - JSON Configuration Example\n";
    std::cout << "======================================\n\n";

    // Method 1: Load from JSON file
    std::cout << "Method 1: Load from JSON file\n";
    std::cout << "-----------------------------\n";
    
    OopParser parser1;
    if (parser1.loadFromJson("../examples/config_template.json")) {
        std::cout << "✓ Configuration loaded successfully\n";
        std::cout << "  Sections: " << parser1.getSectionCount() << "\n";
        
        // Get and print as JSON
        std::cout << "\nConfiguration (JSON format):\n";
        std::cout << parser1.toJsonString(2) << "\n";
    } else {
        std::cout << "✗ Failed to load: " << parser1.getLastError() << "\n";
    }

    // Method 2: Load from JSON string
    std::cout << "\n\nMethod 2: Load from JSON string\n";
    std::cout << "------------------------------\n";
    
    std::string configJson = R"({
      "object": {
        "id": "2025 PA",
        "name": "Asteroid 2025 PA"
      },
      "observer": {
        "latitude": 45.5,
        "longitude": 9.2,
        "site_name": "Milan Observatory"
      },
      "search": {
        "max_magnitude": 18.0,
        "search_radius_deg": 5.0
      }
    })";
    
    OopParser parser2;
    if (parser2.loadFromJsonString(configJson)) {
        std::cout << "✓ Configuration loaded from JSON string\n";
        std::cout << "  Sections: " << parser2.getSectionCount() << "\n";
        
        // Access specific parameters
        auto observer_section = parser2.getSection("observer");
        if (observer_section) {
            auto latitude = observer_section->getParameter("latitude");
            if (latitude) {
                std::cout << "  Observer latitude: " << latitude->asString() << "°\n";
            }
        }
    }

    // Method 3: Build configuration programmatically and output as JSON
    std::cout << "\n\nMethod 3: Build and output as JSON\n";
    std::cout << "---------------------------------\n";
    
    OopParser parser3;
    parser3.setParameter("object", "id", "'Apophis'");
    parser3.setParameter("object", "name", "'99942 Apophis'");
    parser3.setParameter("time", "start_date", "'2029-04-12'");
    parser3.setParameter("time", "end_date", "'2029-04-15'");
    parser3.setParameter("search", "max_magnitude", "8.0");
    
    std::cout << "Configuration as JSON:\n";
    std::cout << parser3.toJsonString(2) << "\n";

    // Method 4: Convert between formats
    std::cout << "\n\nMethod 4: Convert to OOP format\n";
    std::cout << "------------------------------\n";
    
    if (parser3.saveToOop("output_config.oop")) {
        std::cout << "✓ Configuration saved to OOP format (output_config.oop)\n";
    }

    if (parser3.saveToJson("output_config.json")) {
        std::cout << "✓ Configuration saved to JSON format (output_config.json)\n";
    }

    std::cout << "\n======================================\n";
    std::cout << "Example completed successfully!\n";

    return 0;
}
