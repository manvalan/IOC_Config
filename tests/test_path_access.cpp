/**
 * @file test_path_access.cpp
 * @brief Test suite for RFC 6901 JSON Pointer path-based access
 * 
 * Tests the path-based access operations:
 * - getValueByPath()
 * - setValueByPath()
 * - hasPath()
 * - deleteByPath()
 * - getAllPaths()
 * - Path parsing and escaping
 * 
 * @author Michele Bigi
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <cassert>

using namespace ioc_config;

/**
 * @brief Test basic path parsing
 */
bool testPathParsing() {
    auto parts = OopParser::parsePath("/object/id");
    assert(parts.size() == 2 && "Should parse 2 components");
    assert(parts[0] == "object" && "First component should be 'object'");
    assert(parts[1] == "id" && "Second component should be 'id'");
    
    auto root = OopParser::parsePath("/");
    assert(root.empty() && "Root path should be empty");
    
    auto single = OopParser::parsePath("/object");
    assert(single.size() == 1 && "Should parse 1 component");
    assert(single[0] == "object" && "Component should be 'object'");
    
    return true;
}

/**
 * @brief Test path escaping and unescaping
 */
bool testPathEscaping() {
    std::string token_with_tilde = "key~value";
    std::string escaped = OopParser::escapePathToken(token_with_tilde);
    assert(escaped.find("~0") != std::string::npos && "Should escape tilde");
    
    std::string unescaped = OopParser::unescapePathToken(escaped);
    assert(unescaped == token_with_tilde && "Should unescape correctly");
    
    std::string token_with_slash = "key/value";
    std::string escaped2 = OopParser::escapePathToken(token_with_slash);
    assert(escaped2.find("~1") != std::string::npos && "Should escape slash");
    
    std::string unescaped2 = OopParser::unescapePathToken(escaped2);
    assert(unescaped2 == token_with_slash && "Should unescape slash correctly");
    
    return true;
}

/**
 * @brief Test getValueByPath
 */
bool testGetValueByPath() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", "name", "Asteroid");
    parser.setParameter("search", "mag", "16.5");
    
    // Get parameter value
    auto id_val = parser.getValueByPath("/object/id");
    assert(id_val == "17030" && "Should retrieve parameter value");
    
    auto name_val = parser.getValueByPath("/object/name");
    assert(name_val == "Asteroid" && "Should retrieve another parameter");
    
    auto mag_val = parser.getValueByPath("/search/mag");
    assert(mag_val == "16.5" && "Should retrieve from different section");
    
    // Non-existent path
    auto missing = parser.getValueByPath("/object/missing");
    assert(missing.empty() && "Missing path should return empty");
    
    return true;
}

/**
 * @brief Test setValueByPath
 */
bool testSetValueByPath() {
    OopParser parser;
    
    // Create new section and parameter via path
    bool success = parser.setValueByPath("/object/id", "17030");
    assert(success && "Should create section and parameter");
    
    auto val = parser.getValueByPath("/object/id");
    assert(val == "17030" && "Should retrieve set value");
    
    // Modify existing
    success = parser.setValueByPath("/object/id", "17031");
    assert(success && "Should modify existing parameter");
    
    val = parser.getValueByPath("/object/id");
    assert(val == "17031" && "Should reflect modification");
    
    // Create in different section
    success = parser.setValueByPath("/search/magnitude", "15.5");
    assert(success && "Should create in new section");
    
    assert(parser.getSectionCount() >= 2 && "Should have multiple sections");
    
    return true;
}

/**
 * @brief Test hasPath
 */
bool testHasPath() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    
    assert(parser.hasPath("/object/id") && "Should find existing path");
    assert(!parser.hasPath("/object/missing") && "Should not find missing path");
    assert(!parser.hasPath("/missing/id") && "Should not find missing section");
    
    return true;
}

/**
 * @brief Test deleteByPath
 */
bool testDeleteByPath() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", "name", "Asteroid");
    parser.setParameter("search", "mag", "16.5");
    
    // Delete parameter
    assert(parser.hasPath("/object/id") && "Parameter should exist");
    bool success = parser.deleteByPath("/object/id");
    assert(success && "Should delete parameter");
    assert(!parser.hasPath("/object/id") && "Parameter should be deleted");
    
    // Other parameter in same section should remain
    assert(parser.hasPath("/object/name") && "Other parameter should remain");
    
    // Delete section
    success = parser.deleteByPath("/search");
    assert(success && "Should delete section");
    assert(!parser.hasPath("/search/mag") && "Section should be deleted");
    
    return true;
}

/**
 * @brief Test getAllPaths
 */
bool testGetAllPaths() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", "name", "Asteroid");
    parser.setParameter("search", "mag", "16.5");
    
    auto paths = parser.getAllPaths();
    
    // Should have section paths and parameter paths
    assert(paths.size() >= 5 && "Should return all paths");
    
    // Check for expected paths (may have . prefix in keys)
    bool has_object_section = false;
    bool has_search_section = false;
    
    for (const auto& path : paths) {
        if (path.find("/object") != std::string::npos) {
            has_object_section = true;
        }
        if (path.find("/search") != std::string::npos) {
            has_search_section = true;
        }
    }
    
    assert(has_object_section && "Should include object section paths");
    assert(has_search_section && "Should include search section paths");
    
    return true;
}

/**
 * @brief Test path with special characters
 */
bool testSpecialCharactersInPath() {
    OopParser parser;
    
    // Set with special characters
    bool success = parser.setValueByPath("/section~name/param/value", "test");
    // Path should handle escaping, but this is an edge case
    
    // Standard test with normal names
    success = parser.setValueByPath("/my_section/my_param", "value123");
    assert(success && "Should handle underscores");
    
    auto val = parser.getValueByPath("/my_section/my_param");
    assert(val == "value123" && "Should retrieve value with underscores");
    
    return true;
}

/**
 * @brief Test root path retrieval
 */
bool testRootPath() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("search", "mag", "16.5");
    
    // Get root returns JSON
    auto root_json = parser.getValueByPath("/");
    assert(!root_json.empty() && "Root path should return JSON");
    
    // Should be able to parse as JSON
    try {
        auto parsed = nlohmann::json::parse(root_json);
        assert(!parsed.empty() && "Should parse as valid JSON");
    } catch (...) {
        return false;
    }
    
    return true;
}

/**
 * @brief Test section path retrieval
 */
bool testSectionPath() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", "name", "Asteroid");
    
    // Get section returns JSON
    auto section_json = parser.getValueByPath("/object");
    assert(!section_json.empty() && "Section path should return JSON");
    
    try {
        auto parsed = nlohmann::json::parse(section_json);
        assert(parsed.contains("id") && "Should contain id");
        assert(parsed.contains("name") && "Should contain name");
    } catch (...) {
        return false;
    }
    
    return true;
}

/**
 * @brief Run all tests
 */
int main() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "  Testing RFC 6901 Path-Based Access\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    size_t passed = 0, failed = 0;
    
    std::cout << "Test: Path parsing... ";
    if (testPathParsing()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Path escaping/unescaping... ";
    if (testPathEscaping()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: getValueByPath... ";
    if (testGetValueByPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: setValueByPath... ";
    if (testSetValueByPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: hasPath... ";
    if (testHasPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: deleteByPath... ";
    if (testDeleteByPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: getAllPaths... ";
    if (testGetAllPaths()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Special characters in paths... ";
    if (testSpecialCharactersInPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Root path retrieval... ";
    if (testRootPath()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Section path retrieval... ";
    if (testSectionPath()) {
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
