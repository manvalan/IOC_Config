#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    std::cout << "IOC_Config Library - Basic Usage Example\n";
    std::cout << "Library Version: " << getLibraryVersion() << "\n\n";

    // Create parser instance
    OopParser parser;

    // Example: Create configuration programmatically
    ConfigSectionData object_section;
    object_section.name = "object";
    object_section.type = SectionType::OBJECT;

    ConfigParameter id_param;
    id_param.key = "id";
    id_param.value = "'17030'";
    id_param.type = "string";
    object_section.parameters["id"] = id_param;

    ConfigParameter name_param;
    name_param.key = "name";
    name_param.value = "'17030 Sierks'";
    name_param.type = "string";
    object_section.parameters["name"] = name_param;

    std::cout << "Created object section with parameters:\n";
    std::cout << "  .id = " << id_param.value << "\n";
    std::cout << "  .name = " << name_param.value << "\n\n";

    // Demonstrate type conversion
    std::cout << "Type conversion examples:\n";
    try {
        std::cout << "  id as string: " << id_param.asString() << "\n";
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << "\n";
    }

    return 0;
}
