#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    std::cout << "IOC_Config Library - Config Builder Example\n\n";

    OopParser parser;

    // Set parameters for different sections
    ConfigSectionData object_section;
    object_section.name = "object";
    object_section.type = SectionType::OBJECT;

    // Create asteroid object
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'17030 Sierks'");

    // You can manually build sections and save
    std::cout << "Configuration built successfully!\n";
    std::cout << "Sections: " << parser.getSectionCount() << "\n";

    return 0;
}
