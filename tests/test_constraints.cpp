#include "ioc_config/oop_parser.h"
#include <iostream>
#include <iomanip>

using namespace ioc_config;

void testRangeConstraints() {
    std::cout << "\n=== Test: Range Constraints ===\n";
    
    // Test different range expressions
    struct TestCase {
        std::string expr;
        double test_value;
        bool expected;
    };
    
    std::vector<TestCase> tests = {
        {"1..100", 50, true},
        {"1..100", 0, false},
        {"1..100", 101, false},
        {"d >= 4", 5, true},
        {"d >= 4", 4, true},
        {"d >= 4", 3, false},
        {"d <= 100", 100, true},
        {"d <= 100", 101, false},
        {"5 < d < 30", 15, true},
        {"5 < d < 30", 5, false},
        {"5 < d < 30", 30, false},
        {"d < 1000", 999, true},
        {"d < 1000", 1000, false},
        {"d > 0", 0.1, true},
        {"d > 0", 0, false},
    };
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        RangeConstraint constraint;
        if (constraint.parseExpression(test.expr)) {
            bool result = constraint.isSatisfied(test.test_value);
            bool success = (result == test.expected);
            
            std::cout << std::setw(20) << test.expr 
                     << " value=" << std::setw(6) << test.test_value;
            
            // Debug: show parsed constraint details for double inequalities
            if (test.expr.find('<') != std::string::npos && test.expr.find('<', test.expr.find('<') + 1) != std::string::npos) {
                std::cout << " [min=" << constraint.min_value << " inc=" << constraint.min_inclusive 
                         << ", max=" << constraint.max_value << " inc=" << constraint.max_inclusive << "]";
            }
            
            std::cout << " -> " << (result ? "PASS" : "FAIL")
                     << (success ? " ✓" : " ✗ MISMATCH")
                     << "\n";
            
            if (success) passed++;
            else failed++;
        } else {
            std::cout << "Failed to parse: " << test.expr << " ✗\n";
            failed++;
        }
    }
    
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
}

void testParameterSpecs() {
    std::cout << "\n=== Test: Parameter Specifications ===\n";
    
    // Create parameter with constraint
    ParameterSpec param;
    param.key = "magnitude";
    param.required = false;
    param.description = "Star magnitude";
    param.constraint.parseExpression("d < 20");
    
    std::cout << "Parameter: " << param.key << "\n";
    std::cout << "Required: " << (param.required ? "yes" : "no") << "\n";
    std::cout << "Constraint: " << param.constraint.toString() << "\n";
    
    // Test validation
    std::vector<std::string> test_values = {"5.5", "15", "20", "25"};
    std::cout << "\nValidation results:\n";
    for (const auto& val : test_values) {
        bool valid = param.isValid(val);
        std::cout << "  " << val << " -> " << (valid ? "VALID" : "INVALID") << "\n";
    }
}

void testSectionSpecs() {
    std::cout << "\n=== Test: Section Specifications ===\n";
    
    // Create section spec
    SectionSpec search_section;
    search_section.name = "search";
    search_section.required = true;
    search_section.description = "Occultation search parameters";
    
    // Add parameters
    ParameterSpec max_mag;
    max_mag.key = "max_magnitude";
    max_mag.required = false;
    max_mag.description = "Maximum magnitude";
    max_mag.constraint.parseExpression("d < 20");
    search_section.addParameter(max_mag);
    
    ParameterSpec min_dur;
    min_dur.key = "min_duration_sec";
    min_dur.required = false;
    min_dur.description = "Minimum duration";
    min_dur.constraint.parseExpression("d > 0");
    search_section.addParameter(min_dur);
    
    std::cout << "Section: " << search_section.name << "\n";
    std::cout << "Required: " << (search_section.required ? "yes" : "no") << "\n";
    std::cout << "Parameters: " << search_section.params.size() << "\n";
    for (const auto& [key, param] : search_section.params) {
        std::cout << "  - " << key << " (required: " 
                 << (param.required ? "yes" : "no") << ")\n";
    }
}

void testSchemaValidation() {
    std::cout << "\n=== Test: Schema Validation ===\n";
    
    // Create default schema
    ConfigSchema schema = OopParser::createDefaultSchema();
    
    std::cout << "Schema: " << schema.name << " (v" << schema.version << ")\n";
    std::cout << "Sections: " << schema.sections.size() << "\n";
    for (const auto& [name, spec] : schema.sections) {
        std::cout << "  - " << name << " (required: " 
                 << (spec.required ? "yes" : "no") << ")\n";
    }
    
    // Create parser and load config
    OopParser parser;
    parser.setSchema(schema);
    
    parser.setParameter("object", "id", "'17030'");
    parser.setParameter("object", "name", "'Asteroid 17030'");
    parser.setParameter("time", "start_date", "'2025-11-25'");
    parser.setParameter("time", "end_date", "'2025-12-02'");
    parser.setParameter("search", "max_magnitude", "16.0");
    
    // Validate with schema
    std::vector<std::string> errors;
    if (parser.validateWithSchema(schema, errors)) {
        std::cout << "\n✓ Configuration is valid against schema\n";
    } else {
        std::cout << "\n✗ Validation errors:\n";
        for (const auto& error : errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testConstraintParsing() {
    std::cout << "\n=== Test: Constraint Expression Parsing ===\n";
    
    std::vector<std::string> expressions = {
        "1..100",
        "1..N",
        "d >= 4",
        "d <= 100", 
        "d > 5",
        "d < 30",
        "5 < d < 30",
        "0.001..10"
    };
    
    for (const auto& expr : expressions) {
        RangeConstraint c;
        if (c.parseExpression(expr)) {
            std::cout << std::setw(25) << expr << " -> "
                     << "min=" << c.min_value << " (inclusive=" << c.min_inclusive << "), "
                     << "max=" << c.max_value << " (inclusive=" << c.max_inclusive << ")\n";
        } else {
            std::cout << std::setw(25) << expr << " -> FAILED TO PARSE\n";
        }
    }
}

int main() {
    std::cout << "IOC_Config - Advanced Validation and Constraints Tests\n";
    std::cout << "======================================================\n";
    
    try {
        testConstraintParsing();
        testRangeConstraints();
        testParameterSpecs();
        testSectionSpecs();
        testSchemaValidation();
        
        std::cout << "\n======================================================\n";
        std::cout << "All tests completed! ✓\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << "\n";
        return 1;
    }
}
