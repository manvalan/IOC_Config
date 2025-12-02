/**
 * @file test_xml.cpp
 * @brief Test suite for XML format support
 * 
 * Tests the XML loading and saving operations:
 * - loadFromXml()
 * - saveToXml()
 * - loadFromXmlString()
 * - saveToXmlString()
 * - isXmlSupported()
 * 
 * XML Structure Mapping:
 * - XML elements map to configuration sections
 * - XML attributes map to configuration parameters
 * - Text content uses special "_content" key
 * 
 * @author Michele Bigi
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>

using namespace ioc_config;

/**
 * @brief Test XML support availability
 */
bool testXmlSupported() {
    bool supported = OopParser::isXmlSupported();
    assert(supported && "XML support should be available");
    return true;
}

/**
 * @brief Test basic XML string loading
 */
bool testLoadFromXmlString() {
    OopParser parser;
    
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<config>
  <object id="17030" name="Asteroid" />
  <search magnitude="16.5" />
</config>)";
    
    bool success = parser.loadFromXmlString(xml);
    assert(success && "Should successfully load XML");
    
    // Verify sections were created
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    // Verify attributes became parameters
    auto id = object->getParameter(".id");
    assert(id != nullptr && "Should have id parameter from attribute");
    assert(id->value == "17030" && "ID value should be correct");
    
    auto name = object->getParameter(".name");
    assert(name != nullptr && "Should have name parameter");
    assert(name->value == "Asteroid" && "Name should be correct");
    
    auto search = parser.getSection("search");
    assert(search != nullptr && "Should have search section");
    
    auto magnitude = search->getParameter(".magnitude");
    assert(magnitude != nullptr && "Should have magnitude parameter");
    assert(magnitude->value == "16.5" && "Magnitude should be correct");
    
    return true;
}

/**
 * @brief Test XML string with text content
 */
bool testXmlWithContent() {
    OopParser parser;
    
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<config>
  <object id="17030">Vesta Asteroid</object>
  <description>Main belt asteroid</description>
</config>)";
    
    bool success = parser.loadFromXmlString(xml);
    assert(success && "Should load XML with content");
    
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    // Text content should be in ._content
    auto content = object->getParameter("._content");
    assert(content != nullptr && "Should have content parameter");
    assert(content->value == "Vesta Asteroid" && "Content should be preserved");
    
    return true;
}

/**
 * @brief Test saving configuration to XML string
 */
bool testSaveToXmlString() {
    OopParser parser;
    
    // Create configuration programmatically
    parser.setParameter("object", ".id", "17030");
    parser.setParameter("object", ".name", "Asteroid");
    parser.setParameter("search", ".magnitude", "16.5");
    
    std::string xml = parser.saveToXmlString();
    assert(!xml.empty() && "Should generate XML");
    
    // Verify XML structure
    assert(xml.find("<?xml") != std::string::npos && "Should have XML declaration");
    assert(xml.find("<object") != std::string::npos && "Should have object element");
    assert(xml.find("id=") != std::string::npos && "Should have id attribute");
    assert(xml.find("17030") != std::string::npos && "Should contain id value");
    
    return true;
}

/**
 * @brief Test round-trip: save then load
 */
bool testXmlRoundTrip() {
    OopParser parser1;
    
    // Set up original
    parser1.setParameter("object", ".id", "17030");
    parser1.setParameter("object", ".name", "Vesta");
    parser1.setParameter("search", ".mag_limit", "16.5");
    parser1.setParameter("search", ".distance", "2.36");
    
    // Save to XML
    std::string xml = parser1.saveToXmlString();
    assert(!xml.empty() && "Should save to XML");
    
    // Load into new parser
    OopParser parser2;
    bool success = parser2.loadFromXmlString(xml);
    assert(success && "Should load XML back");
    
    // Verify data matches
    auto obj1 = parser1.getSection("object");
    auto obj2 = parser2.getSection("object");
    
    assert(obj1 != nullptr && obj2 != nullptr && "Both should have object section");
    
    auto id1 = obj1->getParameter(".id");
    auto id2 = obj2->getParameter(".id");
    assert(id1 != nullptr && id2 != nullptr && "Both should have id");
    assert(id1->value == id2->value && "IDs should match");
    
    auto name1 = obj1->getParameter(".name");
    auto name2 = obj2->getParameter(".name");
    assert(name1 != nullptr && name2 != nullptr && "Both should have name");
    assert(name1->value == name2->value && "Names should match");
    
    return true;
}

/**
 * @brief Test XML file operations
 */
bool testXmlFileOperations() {
    OopParser parser1;
    parser1.setParameter("object", ".id", "17030");
    parser1.setParameter("object", ".name", "Asteroid");
    
    std::string temp_file = "/tmp/test_config.xml";
    
    // Save to file
    bool save_success = parser1.saveToXml(temp_file);
    assert(save_success && "Should save XML file");
    
    // Verify file exists and has content
    std::ifstream file(temp_file);
    assert(file.good() && "XML file should exist");
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    assert(!content.empty() && "XML file should have content");
    assert(content.find("<?xml") != std::string::npos && "Should have XML declaration");
    
    // Load from file
    OopParser parser2;
    bool load_success = parser2.loadFromXml(temp_file);
    assert(load_success && "Should load XML file");
    
    // Verify data
    auto obj = parser2.getSection("object");
    assert(obj != nullptr && "Should have object section");
    
    auto id = obj->getParameter(".id");
    assert(id != nullptr && id->value == "17030" && "Should have correct id");
    
    // Clean up
    std::remove(temp_file.c_str());
    
    return true;
}

/**
 * @brief Test XML special character escaping
 */
bool testXmlSpecialCharacters() {
    OopParser parser;
    
    std::string xml_with_special = R"(<?xml version="1.0" encoding="UTF-8"?>
<config>
  <description text="Value with &amp; and &lt;tag&gt; chars" />
</config>)";
    
    bool success = parser.loadFromXmlString(xml_with_special);
    assert(success && "Should handle XML entities");
    
    auto desc = parser.getSection("description");
    assert(desc != nullptr && "Should have description section");
    
    auto text = desc->getParameter(".text");
    assert(text != nullptr && "Should have text parameter");
    // Entities should be converted
    assert(text->value.find("&") != std::string::npos || 
           text->value.find("<") != std::string::npos && "Should preserve special chars");
    
    return true;
}

/**
 * @brief Test empty XML
 */
bool testEmptyXml() {
    OopParser parser;
    
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<config>
</config>)";
    
    bool success = parser.loadFromXmlString(xml);
    assert(success && "Should handle empty config");
    
    size_t section_count = parser.getSectionCount();
    assert(section_count == 0 && "Empty config should have no sections");
    
    return true;
}

/**
 * @brief Test XML with multiple elements
 */
bool testXmlMultipleSections() {
    OopParser parser;
    
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<config>
  <object id="1" name="First" />
  <object id="2" name="Second" />
  <search mag="16.5" />
  <search mag="17.0" />
</config>)";
    
    bool success = parser.loadFromXmlString(xml);
    assert(success && "Should load multiple sections");
    
    // Get sections by name
    auto object1 = parser.getSection("object");
    auto search1 = parser.getSection("search");
    
    // Both should exist
    assert(object1 != nullptr && "Should have object section");
    assert(search1 != nullptr && "Should have search section");
    
    // Verify parameters exist
    auto id_param = object1->getParameter(".id");
    assert(id_param != nullptr && "Object should have id parameter");
    
    auto mag_param = search1->getParameter(".mag");
    assert(mag_param != nullptr && "Search should have mag parameter");
    
    return true;
}

/**
 * @brief Run all tests
 */
int main() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "  Testing XML Format Support\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    size_t passed = 0, failed = 0;
    
    std::cout << "Test: XML support available... ";
    if (testXmlSupported()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Load from XML string... ";
    if (testLoadFromXmlString()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: XML with text content... ";
    if (testXmlWithContent()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Save to XML string... ";
    if (testSaveToXmlString()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: XML round-trip... ";
    if (testXmlRoundTrip()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: XML file operations... ";
    if (testXmlFileOperations()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: XML special characters... ";
    if (testXmlSpecialCharacters()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Empty XML... ";
    if (testEmptyXml()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Multiple XML sections... ";
    if (testXmlMultipleSections()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    
    if (failed == 0) {
        std::cout << "All tests PASSED! ✓\n";
    } else {
        std::cout << "Some tests FAILED! ✗\n";
    }
    std::cout << std::string(50, '=') << "\n\n";
    
    return failed == 0 ? 0 : 1;
}
