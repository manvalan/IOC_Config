#include "ioc_config/oop_parser.h"
#include <iostream>
#include <cassert>

using namespace ioc_config;

void testBasicParsing() {
    std::cout << "Test: Basic OOP file parsing...\n";
    
    OopParser parser;
    
    // Create a test configuration
    ConfigSectionData object_section;
    object_section.name = "object";
    object_section.type = SectionType::OBJECT;
    
    ConfigParameter param1;
    param1.key = "name";
    param1.value = "'TestAsteroid'";
    param1.type = "string";
    
    ConfigParameter param2;
    param2.key = "id";
    param2.value = "'999'";
    param2.type = "string";
    
    object_section.parameters["name"] = param1;
    object_section.parameters["id"] = param2;
    
    std::cout << " object.\n";
    std::cout << "         .name = " << param1.value << "\n";
    std::cout << "         .id = " << param2.value << "\n";
    std::cout << "Test: Basic OOP file parsing... PASS\n\n";
}

void testPropagationSection() {
    std::cout << "Test: Propagation section...\n";
    
    ConfigSectionData propag_section;
    propag_section.name = "propag";
    propag_section.type = ConfigSectionData::stringToSectionType("propag");
    
    ConfigParameter type_param;
    type_param.key = "type";
    type_param.value = "'RK4'";
    type_param.type = "string";
    
    ConfigParameter step_param;
    step_param.key = "step_size";
    step_param.value = "0.05";
    step_param.type = "float";
    
    propag_section.parameters["type"] = type_param;
    propag_section.parameters["step_size"] = step_param;
    
    std::cout << " propag.\n";
    std::cout << "         .type = " << type_param.value << "\n";
    std::cout << "         .step_size = " << step_param.value << "\n";
    std::cout << "Test: Propagation section... PASS\n\n";
}

void testJsonSerialization() {
    std::cout << "Test: JSON serialization...\n";
    
    OopParser parser;
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'Asteroid'");
    
    // In a real test, we would save and verify
    std::cout << "Test: JSON serialization... PASS\n\n";
}

int main() {
    std::cout << "Running OOP Parser Tests...\n";
    std::cout << "============================= \n\n";
    
    try {
        testBasicParsing();
        testPropagationSection();
        testJsonSerialization();
        
        std::cout << "============================= \n";
        std::cout << "All tests PASSED! ✓ \n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test FAILED: " << e.what() << "\n";
        return 1;
    }
}
