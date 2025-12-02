/**
 * @file test_streaming.cpp
 * @brief Tests for Streaming I/O Operations (Phase 2B.2)
 * 
 * @author Michele Bigi
 * @version 1.3.0
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace ioc_config;
namespace fs = std::filesystem;

/**
 * @brief Test load from string stream
 */
bool testLoadFromStringStream() {
    std::string oop_content = R"([section1]
param1 = value1
param2 = 123

[section2]
id = 456
name = TestSection
)";
    
    std::istringstream iss(oop_content);
    OopParser parser;
    
    bool success = parser.loadFromStream(iss);
    assert(success);
    
    // Verify content
    auto sections = parser.getAllSections();
    assert(sections.size() == 2);
    assert(sections[0].name == "section1");
    assert(sections[1].name == "section2");
    
    return true;
}

/**
 * @brief Test save to string stream
 */
bool testSaveToStringStream() {
    OopParser parser;
    
    parser.setParameter("section1", "param1", "value1");
    parser.setParameter("section1", "param2", "123");
    parser.setParameter("section2", "id", "456");
    parser.setParameter("section2", "name", "TestSection");
    
    std::ostringstream oss;
    bool success = parser.saveToStream(oss);
    assert(success);
    
    std::string result = oss.str();
    assert(!result.empty());
    assert(result.find("[section1]") != std::string::npos);
    assert(result.find("[section2]") != std::string::npos);
    assert(result.find("param1 = value1") != std::string::npos);
    
    return true;
}

/**
 * @brief Test stream round-trip (save then load)
 */
bool testStreamRoundTrip() {
    OopParser parser1;
    
    // Create configuration
    parser1.setParameter("objects", "name", "Vesta");
    parser1.setParameter("objects", "id", "17030");
    parser1.setParameter("search", "magnitude", "16.5");
    parser1.setParameter("search", "radius", "1.5");
    
    // Save to stream
    std::ostringstream oss;
    assert(parser1.saveToStream(oss));
    
    // Load from same stream
    std::string content = oss.str();
    std::istringstream iss(content);
    
    OopParser parser2;
    assert(parser2.loadFromStream(iss));
    
    // Verify configuration is identical
    auto sections1 = parser1.getAllSections();
    auto sections2 = parser2.getAllSections();
    
    assert(sections1.size() == sections2.size());
    
    // Check values match
    std::string val1 = parser1.getValueByPath("/objects/name");
    std::string val2 = parser2.getValueByPath("/objects/name");
    assert(val1 == val2);
    assert(val1 == "Vesta");
    
    return true;
}

/**
 * @brief Test load from file stream
 */
bool testLoadFromFileStream() {
    std::string test_dir = "./test_stream_temp";
    if (fs::exists(test_dir)) {
        fs::remove_all(test_dir);
    }
    fs::create_directory(test_dir);
    
    // Create a test file
    std::string filepath = test_dir + "/test.oop";
    std::ofstream outfile(filepath);
    outfile << "[section1]\nparam1 = value1\nparam2 = 123\n";
    outfile << "[section2]\nid = 456\n";
    outfile.close();
    
    // Load from file stream
    std::ifstream infile(filepath);
    OopParser parser;
    bool success = parser.loadFromStream(infile);
    infile.close();
    
    assert(success);
    
    auto sections = parser.getAllSections();
    assert(sections.size() == 2);
    assert(sections[0].name == "section1");
    
    fs::remove_all(test_dir);
    return true;
}

/**
 * @brief Test save to file stream
 */
bool testSaveToFileStream() {
    std::string test_dir = "./test_stream_temp";
    if (fs::exists(test_dir)) {
        fs::remove_all(test_dir);
    }
    fs::create_directory(test_dir);
    
    OopParser parser;
    parser.setParameter("config", "option1", "value1");
    parser.setParameter("config", "option2", "123");
    
    std::string filepath = test_dir + "/output.oop";
    std::ofstream outfile(filepath);
    bool success = parser.saveToStream(outfile);
    outfile.close();
    
    assert(success);
    assert(fs::exists(filepath));
    
    // Verify file content
    std::ifstream infile(filepath);
    std::string content((std::istreambuf_iterator<char>(infile)),
                       std::istreambuf_iterator<char>());
    infile.close();
    
    assert(!content.empty());
    assert(content.find("[config]") != std::string::npos);
    
    fs::remove_all(test_dir);
    return true;
}

/**
 * @brief Test load from stream with empty content
 */
bool testLoadFromStreamEmpty() {
    std::istringstream iss("");
    OopParser parser;
    
    bool success = parser.loadFromStream(iss);
    assert(success);  // Empty stream is valid
    
    auto sections = parser.getAllSections();
    assert(sections.size() == 0);
    
    return true;
}

/**
 * @brief Test load from stream with comments
 */
bool testLoadFromStreamWithComments() {
    std::string content = R"(# This is a comment
[section1]
# Another comment
param1 = value1
; Semicolon comment
param2 = value2

[section2]
id = 123
)";
    
    std::istringstream iss(content);
    OopParser parser;
    
    bool success = parser.loadFromStream(iss);
    assert(success);
    
    auto sections = parser.getAllSections();
    assert(sections.size() == 2);
    
    // Verify parameters were loaded (comments ignored)
    assert(sections[0].parameters.size() == 2);
    
    return true;
}

/**
 * @brief Test load from stream with various data types
 */
bool testLoadFromStreamDataTypes() {
    std::string content = R"([data]
string_val = hello world
int_val = 42
float_val = 3.14159
bool_true = true
bool_false = false
array_val = [1,2,3]
)";
    
    std::istringstream iss(content);
    OopParser parser;
    
    bool success = parser.loadFromStream(iss);
    assert(success);
    
    // Verify type detection
    std::string str_val = parser.getValueByPath("/data/string_val");
    assert(str_val == "hello world");
    
    std::string int_val = parser.getValueByPath("/data/int_val");
    assert(int_val == "42");
    
    return true;
}

/**
 * @brief Test save and load preserve formatting
 */
bool testStreamPreservesContent() {
    std::string original = R"([section1]
name = TestObject
value = 123.456
active = true

[section2]
count = 100
title = Special Section
)";
    
    std::istringstream iss1(original);
    OopParser parser1;
    assert(parser1.loadFromStream(iss1));
    
    std::ostringstream oss;
    assert(parser1.saveToStream(oss));
    
    std::string saved = oss.str();
    std::istringstream iss2(saved);
    OopParser parser2;
    assert(parser2.loadFromStream(iss2));
    
    // Both should have same structure
    auto sec1 = parser1.getAllSections();
    auto sec2 = parser2.getAllSections();
    
    assert(sec1.size() == sec2.size());
    
    for (size_t i = 0; i < sec1.size(); ++i) {
        assert(sec1[i].name == sec2[i].name);
        assert(sec1[i].parameters.size() == sec2[i].parameters.size());
    }
    
    return true;
}

/**
 * @brief Test stream with special characters
 */
bool testStreamSpecialCharacters() {
    std::string content = R"([section]
path = /home/user/config/file.txt
email = user@example.com
equation = x^2 + y^2 = z^2
unicode = café résumé
)";
    
    std::istringstream iss(content);
    OopParser parser;
    
    bool success = parser.loadFromStream(iss);
    assert(success);
    
    auto sections = parser.getAllSections();
    assert(sections.size() == 1);
    assert(sections[0].parameters.size() == 4);
    
    return true;
}

/**
 * @brief Test multiple stream operations
 */
bool testMultipleStreamOperations() {
    OopParser parser1;
    parser1.setParameter("config", "option1", "value1");
    parser1.setParameter("config", "option2", "value2");
    
    // First save
    std::ostringstream oss1;
    assert(parser1.saveToStream(oss1));
    
    // Load from first save
    std::istringstream iss1(oss1.str());
    OopParser parser2;
    assert(parser2.loadFromStream(iss1));
    
    // Modify and save again
    parser2.setParameter("config", "option3", "value3");
    std::ostringstream oss2;
    assert(parser2.saveToStream(oss2));
    
    // Final load
    std::istringstream iss2(oss2.str());
    OopParser parser3;
    assert(parser3.loadFromStream(iss2));
    
    // Verify final state
    auto sections = parser3.getAllSections();
    assert(sections.size() == 1);
    assert(sections[0].parameters.size() == 3);
    
    return true;
}

int main(int argc, char** argv) {
    std::cout << "\n==================================================\n";
    std::cout << "  Testing Streaming I/O Operations (Phase 2B.2)\n";
    std::cout << "==================================================\n\n";
    
    int passed = 0;
    int failed = 0;
    
    // Test cases
    auto runTest = [&](const std::string& name, bool (*test)()) {
        try {
            if (test()) {
                std::cout << "✓ Test: " << name << "... PASS\n";
                passed++;
            } else {
                std::cout << "✗ Test: " << name << "... FAIL\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ Test: " << name << "... EXCEPTION: " << e.what() << "\n";
            failed++;
        }
    };
    
    runTest("Load from string stream", testLoadFromStringStream);
    runTest("Save to string stream", testSaveToStringStream);
    runTest("Stream round-trip", testStreamRoundTrip);
    runTest("Load from file stream", testLoadFromFileStream);
    runTest("Save to file stream", testSaveToFileStream);
    runTest("Load from stream empty", testLoadFromStreamEmpty);
    runTest("Load from stream with comments", testLoadFromStreamWithComments);
    runTest("Load from stream data types", testLoadFromStreamDataTypes);
    runTest("Stream preserves content", testStreamPreservesContent);
    runTest("Stream special characters", testStreamSpecialCharacters);
    runTest("Multiple stream operations", testMultipleStreamOperations);
    
    std::cout << "\n==================================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    if (failed == 0) {
        std::cout << "All tests PASSED! ✓\n";
    } else {
        std::cout << "Some tests FAILED!\n";
    }
    std::cout << "==================================================\n\n";
    
    return (failed == 0) ? 0 : 1;
}
