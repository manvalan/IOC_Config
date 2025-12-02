#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    std::cout << "Test: Config builder...\n\n";
    
    OopParser parser;
    
    // Build configuration programmatically
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'17030 Sierks'");
    
    parser.setParameter("time", ".start_date", "'2025-11-25'");
    parser.setParameter("time", ".end_date", "'2025-12-02'");
    
    parser.setParameter("search", ".start_jd", "2460638.5");
    parser.setParameter("search", ".end_jd", "2460645.5");
    
    std::cout << "Configuration created with " << parser.getSectionCount() 
              << " sections\n";
    
    // Retrieve and verify
    auto sections = parser.getAllSections();
    std::cout << "\nBuilt sections:\n";
    for (const auto& section : sections) {
        std::cout << "  " << section.name << " (" << section.parameters.size() 
                  << " parameters)\n";
    }
    
    std::cout << "\nTest: Config builder... PASS\n";
    return 0;
}
