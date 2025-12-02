/**
 * @file test_versioning.cpp
 * @brief Unit tests for Phase 2B.3: Versioning functionality
 */
#include <cassert>
#include <iostream>
#include "../include/ioc_config/oop_parser.h"

using namespace ioc_config;

int main() {
    int testCount = 0, passCount = 0;
    
    std::cout << "\n=== Phase 2B.3: Versioning Tests ===" << std::endl;
    
    // Test 1: Enable/Disable
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Enable/Disable ... ";
        VersionedOopParser p;
        assert(!p.isVersioningEnabled());
        assert(p.enableVersioning("Init"));
        assert(p.isVersioningEnabled());
        assert(p.disableVersioning());
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 2: Create versions
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Create versions ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        p.createVersion("V2");
        p.createVersion("V3");
        assert(p.getVersionCount() == 3);
        assert(p.getCurrentVersion() == 3);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 3: Get history
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Get history ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        p.createVersion("V2");
        auto h = p.getHistory();  // Copia, non reference
        assert(h.size() == 2);
        assert(h[0].version == 1);
        assert(h[1].version == 2);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 4: Rollback
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Rollback ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v1");
        p.enableVersioning("V1");
        p.createVersion("V2");
        p.setParameter("s", "p", "v2");
        assert(p.rollback(1));
        auto* cp = p.findParameter("p");
        assert(cp != nullptr);
        assert(cp->value == "v1");
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 5: Rollback previous
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Rollback previous ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        p.createVersion("V2");
        p.createVersion("V3");
        assert(p.rollbackPrevious());
        assert(p.getCurrentVersion() == 2);
        assert(p.rollbackPrevious());
        assert(p.getCurrentVersion() == 1);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 6: Invalid rollback
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Invalid rollback ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        assert(!p.rollback(99));
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 7: Clear history
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Clear history ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        p.createVersion("V2");
        p.createVersion("V3");
        assert(p.clearHistory());
        assert(p.getVersionCount() == 1);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 8: Disabled operations
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Disabled operations ... ";
        VersionedOopParser p;
        assert(!p.createVersion("fail"));
        assert(!p.rollback(1));
        assert(p.getVersionCount() == 0);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 9: Snapshots independent
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Snapshots independent ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "orig");
        p.enableVersioning("V1");
        p.createVersion("V2");
        p.setParameter("s", "p", "changed");
        assert(p.rollback(1));
        auto* cp = p.findParameter("p");
        assert(cp != nullptr);
        assert(cp->value == "orig");
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 10: JSON export
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": JSON export ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        p.createVersion("V2");
        auto j = p.getHistoryAsJson();
        assert(j.is_array());
        assert(j.size() == 2);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 11: Version descriptions
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Version descriptions ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("Desc1");
        p.createVersion("Desc2");
        assert(p.getVersionDescription(1) == "Desc1");
        assert(p.getVersionDescription(2) == "Desc2");
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    // Test 12: Version timestamps
    try {
        ++testCount;
        std::cout << "Test " << testCount << ": Version timestamps ... ";
        VersionedOopParser p;
        p.setParameter("s", "p", "v");
        p.enableVersioning("V1");
        auto ts = p.getVersionTimestamp(1);
        assert(!ts.empty());
        assert(ts.find("T") != std::string::npos);
        assert(ts.find("Z") != std::string::npos);
        std::cout << "PASS" << std::endl; ++passCount;
    } catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << std::endl; }
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Total: " << testCount << " | Pass: " << passCount << " | Fail: " << (testCount - passCount) << std::endl;
    return passCount == testCount ? 0 : 1;
}
