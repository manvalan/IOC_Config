/**
 * @file test_csv.cpp
 * @brief Test suite for CSV format support
 * 
 * Tests the CSV loading and saving operations:
 * - loadFromCsv()
 * - saveToCsv()
 * - loadFromCsvString()
 * - saveToCsvString()
 * - detectCsvDelimiter()
 * 
 * CSV Structure Mapping:
 * - First column: section name
 * - Subsequent columns: parameter names (from header)
 * - Values: parameter values
 * - Headers: parameter names with "." prefix
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
 * @brief Test CSV delimiter detection
 */
bool testDelimiterDetection() {
    // Comma-delimited
    std::string csv_comma = "name,id,type\nobject,17030,asteroid";
    assert(OopParser::detectCsvDelimiter(csv_comma) == ',' && "Should detect comma");
    
    // Semicolon-delimited
    std::string csv_semicolon = "name;id;type\nobject;17030;asteroid";
    assert(OopParser::detectCsvDelimiter(csv_semicolon) == ';' && "Should detect semicolon");
    
    // Tab-delimited
    std::string csv_tab = "name\tid\ttype\nobject\t17030\tasteroid";
    assert(OopParser::detectCsvDelimiter(csv_tab) == '\t' && "Should detect tab");
    
    return true;
}

/**
 * @brief Test basic CSV string loading
 */
bool testLoadFromCsvString() {
    OopParser parser;
    
    std::string csv = R"(section,id,name,type
object,17030,Vesta,asteroid
search,16.5,magnitude,limit)";
    
    bool success = parser.loadFromCsvString(csv, true);
    assert(success && "Should successfully load CSV");
    
    // Verify sections were created
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    auto id = object->getParameter(".id");
    assert(id != nullptr && "Should have id parameter");
    assert(id->value == "17030" && "ID value should be correct");
    
    auto search = parser.getSection("search");
    assert(search != nullptr && "Should have search section");
    
    auto mag = search->getParameter(".magnitude");
    assert(mag != nullptr && "Should have magnitude parameter");
    assert(mag->value == "16.5" && "Magnitude should be correct");
    
    return true;
}

/**
 * @brief Test CSV without header
 */
bool testCsvNoHeader() {
    OopParser parser;
    
    std::string csv = R"(object,17030,Vesta
search,16.5,magnitude)";
    
    bool success = parser.loadFromCsvString(csv, false);
    assert(success && "Should load CSV without header");
    
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    // Without header, columns won't have meaningful names
    // but section should be created
    assert(object->getParameterCount() > 0 && "Should have parameters");
    
    return true;
}

/**
 * @brief Test CSV with quoted fields
 */
bool testCsvQuotedFields() {
    OopParser parser;
    
    std::string csv = R"(section,description,value
object,"Contains, comma",17030
search,"Text with ""quotes""",16.5)";
    
    bool success = parser.loadFromCsvString(csv, true);
    assert(success && "Should handle quoted fields");
    
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    auto desc = object->getParameter(".description");
    assert(desc != nullptr && "Should have description");
    // Quotes should be preserved
    assert(desc->value.find("comma") != std::string::npos && "Should preserve field content");
    
    return true;
}

/**
 * @brief Test saving configuration to CSV string
 */
bool testSaveToCSvString() {
    OopParser parser;
    
    // Create configuration programmatically
    parser.setParameter("object", ".id", "17030");
    parser.setParameter("object", ".name", "Vesta");
    parser.setParameter("search", ".magnitude", "16.5");
    
    std::string csv = parser.saveToCsvString(true);
    assert(!csv.empty() && "Should generate CSV");
    
    // Verify CSV structure
    assert(csv.find("Section") != std::string::npos && "Should have header");
    assert(csv.find("object") != std::string::npos && "Should have object section");
    assert(csv.find("17030") != std::string::npos && "Should contain id value");
    assert(csv.find("search") != std::string::npos && "Should have search section");
    
    return true;
}

/**
 * @brief Test round-trip: save then load
 */
bool testCsvRoundTrip() {
    OopParser parser1;
    
    // Set up original
    parser1.setParameter("object", ".id", "17030");
    parser1.setParameter("object", ".name", "Vesta");
    parser1.setParameter("search", ".mag", "16.5");
    parser1.setParameter("search", ".type", "asteroid");
    
    // Save to CSV
    std::string csv = parser1.saveToCsvString(true);
    assert(!csv.empty() && "Should save to CSV");
    
    // Load into new parser
    OopParser parser2;
    bool success = parser2.loadFromCsvString(csv, true);
    assert(success && "Should load CSV back");
    
    // Verify data matches
    auto obj1 = parser1.getSection("object");
    auto obj2 = parser2.getSection("object");
    
    assert(obj1 != nullptr && obj2 != nullptr && "Both should have object section");
    
    auto id1 = obj1->getParameter(".id");
    auto id2 = obj2->getParameter(".id");
    assert(id1 != nullptr && id2 != nullptr && "Both should have id");
    assert(id1->value == id2->value && "IDs should match");
    
    return true;
}

/**
 * @brief Test CSV file operations
 */
bool testCsvFileOperations() {
    OopParser parser1;
    parser1.setParameter("object", ".id", "17030");
    parser1.setParameter("object", ".name", "Asteroid");
    
    std::string temp_file = "/tmp/test_config.csv";
    
    // Save to file
    bool save_success = parser1.saveToCsv(temp_file, true);
    assert(save_success && "Should save CSV file");
    
    // Verify file exists and has content
    std::ifstream file(temp_file);
    assert(file.good() && "CSV file should exist");
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    assert(!content.empty() && "CSV file should have content");
    assert(content.find("Section") != std::string::npos && "Should have header");
    
    // Load from file
    OopParser parser2;
    bool load_success = parser2.loadFromCsv(temp_file, true);
    assert(load_success && "Should load CSV file");
    
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
 * @brief Test CSV with special characters
 */
bool testCsvSpecialCharacters() {
    OopParser parser;
    
    std::string csv = R"(section,description
object,Value with commas, and stuff
search,Contains "quotes" inside)";
    
    bool success = parser.loadFromCsvString(csv, true);
    assert(success && "Should handle special characters");
    
    auto object = parser.getSection("object");
    assert(object != nullptr && "Should have object section");
    
    auto desc = object->getParameter(".description");
    assert(desc != nullptr && "Should have description parameter");
    
    return true;
}

/**
 * @brief Test empty CSV
 */
bool testEmptyCsv() {
    OopParser parser;
    
    std::string csv = "";
    
    bool success = parser.loadFromCsvString(csv, true);
    assert(!success && "Empty CSV should fail");
    
    return true;
}

/**
 * @brief Test CSV with multiple rows same section
 */
bool testCsvMultipleRows() {
    OopParser parser;
    
    std::string csv = R"(section,id,name
object,1,First
object,2,Second
search,16.5,mag)";
    
    bool success = parser.loadFromCsvString(csv, true);
    assert(success && "Should handle multiple rows");
    
    // Should have both object and search sections
    auto object = parser.getSection("object");
    auto search = parser.getSection("search");
    
    assert(object != nullptr && "Should have object section");
    assert(search != nullptr && "Should have search section");
    
    return true;
}

/**
 * @brief Test CSV without header option
 */
bool testCsvHeaderOption() {
    OopParser parser1, parser2;
    
    std::string csv = R"(object,17030,Vesta)";
    
    // Load without header
    bool success1 = parser1.loadFromCsvString(csv, false);
    assert(success1 && "Should load without header");
    
    auto obj = parser1.getSection("object");
    assert(obj != nullptr && "Should create section");
    
    return true;
}

/**
 * @brief Run all tests
 */
int main() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "  Testing CSV Format Support\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    size_t passed = 0, failed = 0;
    
    std::cout << "Test: CSV delimiter detection... ";
    if (testDelimiterDetection()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Load from CSV string... ";
    if (testLoadFromCsvString()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV without header... ";
    if (testCsvNoHeader()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV with quoted fields... ";
    if (testCsvQuotedFields()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Save to CSV string... ";
    if (testSaveToCSvString()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV round-trip... ";
    if (testCsvRoundTrip()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV file operations... ";
    if (testCsvFileOperations()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV special characters... ";
    if (testCsvSpecialCharacters()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Empty CSV... ";
    if (testEmptyCsv()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV multiple rows... ";
    if (testCsvMultipleRows()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: CSV header option... ";
    if (testCsvHeaderOption()) {
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
