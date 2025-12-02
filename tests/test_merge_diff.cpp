/**
 * @file test_merge_diff.cpp
 * @brief Test suite for Merge, Diff, Clone, and Query operations
 * 
 * Tests the advanced configuration operations:
 * - Merge with different strategies
 * - Diff comparison
 * - Clone and copy operations
 * - Query and filtering
 * 
 * @author Michele Bigi
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <cassert>
#include <sstream>

using namespace ioc_config;

/**
 * @brief Test merge with REPLACE strategy
 */
bool testMergeReplace() {
    OopParser parser1, parser2;
    
    // Parser 1: original configuration
    parser1.setParameter("object", "id", "17030");
    parser1.setParameter("object", "name", "Asteroid17030");
    parser1.setParameter("search", "mag", "16.5");
    
    // Parser 2: new configuration with some overlaps
    parser2.setParameter("object", "id", "17031");  // Override
    parser2.setParameter("object", "type", "NEO");  // New
    parser2.setParameter("search", "mag", "17.0");  // Override
    parser2.setParameter("output", "format", "JSON");  // New section
    
    // Merge with REPLACE strategy
    bool success = parser1.merge(parser2, MergeStrategy::REPLACE);
    assert(success && "Merge should succeed");
    
    // Check results
    auto section = parser1.getSection("object");
    assert(section != nullptr && "object section should exist");
    
    auto id = section->getParameter("id");
    assert(id != nullptr && id->value == "17031" && "id should be replaced");
    
    auto type = section->getParameter("type");
    assert(type != nullptr && type->value == "NEO" && "type should be added");
    
    auto name = section->getParameter("name");
    assert(name != nullptr && name->value == "Asteroid17030" && "name should be preserved");
    
    auto output = parser1.getSection("output");
    assert(output != nullptr && "output section should be added");
    
    // Check merge stats
    const auto& stats = parser1.getLastMergeStats();
    assert(stats.sections_added >= 1 && "Should add output section");
    
    // Verify search section also has the merged parameter
    auto search = parser1.getSection("search");
    assert(search != nullptr && "search section should exist");
    auto mag = search->getParameter("mag");
    assert(mag != nullptr && mag->value == "17.0" && "mag should be replaced");
    
    return true;
}

/**
 * @brief Test merge with APPEND strategy
 */
bool testMergeAppend() {
    OopParser parser1, parser2;
    
    parser1.setParameter("object", "id", "17030");
    parser1.setParameter("object", "name", "Original");
    
    parser2.setParameter("object", "id", "SHOULD_NOT_OVERRIDE");
    parser2.setParameter("object", "type", "NEW");
    
    bool success = parser1.merge(parser2, MergeStrategy::APPEND);
    assert(success && "APPEND merge should succeed");
    
    // Check that id wasn't overridden
    auto section = parser1.getSection("object");
    auto id = section->getParameter("id");
    assert(id->value == "17030" && "id should NOT be overridden in APPEND mode");
    
    // Check that new parameter was added
    auto type = section->getParameter("type");
    assert(type != nullptr && type->value == "NEW" && "type should be added");
    
    return true;
}

/**
 * @brief Test diff functionality
 */
bool testDiff() {
    OopParser config1, config2;
    
    // Config 1
    config1.setParameter("object", "id", "17030");
    config1.setParameter("object", "name", "Asteroid");
    config1.setParameter("search", "mag", "16.5");
    
    // Config 2
    config2.setParameter("object", "id", "17030");  // Same
    config2.setParameter("object", "name", "Asteroid 17030");  // Different
    config2.setParameter("object", "type", "NEO");  // New
    config2.setParameter("search", "mag", "17.0");  // Different
    
    auto diffs = config1.diff(config2);
    
    // Should have: 1 modified (name), 1 added (type), 1 modified (mag), 1 unchanged (id)
    size_t modified = 0, added = 0, removed = 0, unchanged = 0;
    
    for (const auto& diff : diffs) {
        if (diff.type == DiffEntry::MODIFIED) modified++;
        else if (diff.type == DiffEntry::ADDED) added++;
        else if (diff.type == DiffEntry::REMOVED) removed++;
        else if (diff.type == DiffEntry::UNCHANGED) unchanged++;
    }
    
    assert(modified >= 2 && "Should have 2 modifications");
    assert(added == 1 && "Should have 1 addition (type)");
    
    return true;
}

/**
 * @brief Test diff report generation
 */
bool testDiffReport() {
    OopParser config1, config2;
    
    config1.setParameter("object", "id", "17030");
    config2.setParameter("object", "id", "17031");
    config2.setParameter("object", "name", "New");
    
    std::string report = config1.diffReport(config2, true);  // Only changes
    
    assert(report.find("Modified") != std::string::npos && "Report should mention modifications");
    assert(report.find("Added") != std::string::npos && "Report should mention additions");
    assert(report.find("Summary") != std::string::npos && "Report should have summary");
    
    return true;
}

/**
 * @brief Test diff as JSON
 */
bool testDiffJson() {
    OopParser config1, config2;
    
    config1.setParameter("object", "id", "17030");
    config2.setParameter("object", "id", "17031");
    
    auto json_diff = config1.diffAsJson(config2);
    
    assert(json_diff.is_array() && "Diff should be JSON array");
    assert(json_diff.size() > 0 && "Diff should have entries");
    
    return true;
}

/**
 * @brief Test clone functionality
 */
bool testClone() {
    OopParser original;
    original.setParameter("object", "id", "17030");
    original.setParameter("object", "name", "Test");
    original.setParameter("search", "mag", "16.5");
    
    auto cloned = original.clone();
    
    assert(cloned != nullptr && "Clone should not be null");
    assert(cloned->getSectionCount() == original.getSectionCount() && "Sections should match");
    
    // Verify content
    auto orig_section = original.getSection("object");
    auto clone_section = cloned->getSection("object");
    
    assert(orig_section->getParameter("id")->value == 
           clone_section->getParameter("id")->value && "Values should match");
    
    // Modify clone and verify original is unchanged
    cloned->setParameter("object", "id", "MODIFIED");
    assert(original.getSection("object")->getParameter("id")->value == "17030" &&
           "Original should not be affected");
    
    return true;
}

/**
 * @brief Test copyFrom functionality
 */
bool testCopyFrom() {
    OopParser source, dest;
    
    source.setParameter("object", "id", "17030");
    source.setParameter("object", "name", "Source");
    
    dest.copyFrom(source);
    
    assert(dest.getSectionCount() > 0 && "Dest should have sections after copy");
    assert(dest.getSection("object")->getParameter("id")->value == "17030" &&
           "Content should be copied");
    
    return true;
}

/**
 * @brief Test isEmpty functionality
 */
bool testIsEmpty() {
    OopParser empty, nonempty;
    
    assert(empty.isEmpty() && "New parser should be empty");
    
    nonempty.setParameter("object", "id", "123");
    assert(!nonempty.isEmpty() && "Parser with data should not be empty");
    
    return true;
}

/**
 * @brief Test getParametersWhere with predicate
 */
bool testGetParametersWhere() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", ".magnitude", "16.5");
    parser.setParameter("search", ".max_mag", "17.0");
    
    // Find all numeric-looking parameters
    auto numeric = parser.getParametersWhere([](const ConfigParameter& p) {
        try {
            std::stod(p.value);
            return true;
        } catch (...) {
            return false;
        }
    });
    
    assert(numeric.size() >= 2 && "Should find numeric parameters");
    
    return true;
}

/**
 * @brief Test getParametersByKeyPattern
 */
bool testGetParametersByKeyPattern() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", ".magnitude", "16.5");
    parser.setParameter("search", ".max_magnitude", "17.0");
    
    auto mag_params = parser.getParametersByKeyPattern(".*magnitude.*");
    assert(mag_params.size() >= 2 && "Should find parameters matching pattern");
    
    return true;
}

/**
 * @brief Test getParametersByValuePattern
 */
bool testGetParametersByValuePattern() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", "name", "Asteroid");
    parser.setParameter("search", ".description", "Asteroid data");
    
    auto asteroid_params = parser.getParametersByValuePattern(".*steroid.*");
    assert(asteroid_params.size() >= 2 && "Should find parameters with matching values");
    
    return true;
}

/**
 * @brief Test getParametersByType
 */
bool testGetParametersByType() {
    OopParser parser;
    
    parser.setParameter("object", "id", "17030");
    parser.setParameter("object", ".magnitude", "16.5");
    parser.setParameter("object", ".status", ".TRUE.");
    
    auto int_params = parser.getParametersByType("int");
    assert(int_params.size() >= 1 && "Should find int parameters");
    
    auto float_params = parser.getParametersByType("float");
    assert(float_params.size() >= 1 && "Should find float parameters");
    
    return true;
}

/**
 * @brief Run all tests
 */
int main() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "  Testing Merge, Diff, Clone & Query Operations\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    size_t passed = 0, failed = 0;
    
    // Merge tests
    std::cout << "Test: Merge with REPLACE strategy... ";
    if (testMergeReplace()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Merge with APPEND strategy... ";
    if (testMergeAppend()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    // Diff tests
    std::cout << "Test: Diff functionality... ";
    if (testDiff()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Diff report generation... ";
    if (testDiffReport()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: Diff as JSON... ";
    if (testDiffJson()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    // Clone tests
    std::cout << "Test: Clone functionality... ";
    if (testClone()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: copyFrom functionality... ";
    if (testCopyFrom()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: isEmpty functionality... ";
    if (testIsEmpty()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    // Query tests
    std::cout << "Test: getParametersWhere with predicate... ";
    if (testGetParametersWhere()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: getParametersByKeyPattern... ";
    if (testGetParametersByKeyPattern()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: getParametersByValuePattern... ";
    if (testGetParametersByValuePattern()) {
        std::cout << "PASS\n";
        passed++;
    } else {
        std::cout << "FAIL\n";
        failed++;
    }
    
    std::cout << "Test: getParametersByType... ";
    if (testGetParametersByType()) {
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
