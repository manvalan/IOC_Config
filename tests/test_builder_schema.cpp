#include "ioc_config/oop_parser.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace ioc_config;

void testConfigBuilder() {
    std::cout << "\n=== Test: ConfigBuilder Fluent API ===\n";
    
    try {
        // Build a configuration using fluent API
        ConfigBuilder builder;
        builder.addSection("object")
               .addParameter("id", "17030")
               .addParameter("name", "Asteroid 17030")
               .addParameter("type", "asteroid")
               .endSection()
               .addSection("time")
               .addParameter("start_date", "2025-12-01")
               .addParameter("end_date", "2025-12-31")
               .endSection()
               .addSection("search")
               .addParameter("max_magnitude", "16.5")
               .addParameter("min_duration_sec", "0.1")
               .endSection();
        
        // Get the built parser
        auto parser = builder.build();
        
        // Verify sections
        assert(builder.getSectionCount() == 3);
        std::cout << "✓ Created " << builder.getSectionCount() << " sections\n";
        
        // Verify parameters
        auto object_section = parser->getSection("object");
        assert(object_section != nullptr);
        assert(object_section->getParameter("id") != nullptr);
        assert(object_section->getParameter("id")->asString() == "17030");
        std::cout << "✓ Object section has correct parameters\n";
        
        auto time_section = parser->getSection("time");
        assert(time_section != nullptr);
        assert(time_section->parameters.size() == 2);
        std::cout << "✓ Time section has 2 parameters\n";
        
        // Print section names
        std::cout << "\nSection names: ";
        auto names = builder.getSectionNames();
        for (const auto& name : names) {
            std::cout << name << " ";
        }
        std::cout << "\n";
        
        std::cout << "\nTest: ConfigBuilder... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testBuilderMethodChaining() {
    std::cout << "\n=== Test: Method Chaining ===\n";
    
    try {
        // Test fluent interface
        ConfigBuilder builder;
        auto& ref1 = builder.addSection("object");
        auto& ref2 = ref1.addParameter("id", "123");
        auto& ref3 = ref2.addParameter("name", "Test");
        auto& ref4 = ref3.endSection();
        
        // Verify it returns the same builder
        assert(&ref1 == &builder);
        assert(&ref2 == &builder);
        assert(&ref3 == &builder);
        assert(&ref4 == &builder);
        
        std::cout << "✓ Method chaining works correctly\n";
        std::cout << "Test: Method Chaining... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testBuilderClear() {
    std::cout << "\n=== Test: Builder Clear ===\n";
    
    try {
        ConfigBuilder builder;
        builder.addSection("object").addParameter("id", "1").endSection();
        assert(builder.getSectionCount() == 1);
        
        builder.clear();
        assert(builder.getSectionCount() == 0);
        
        builder.addSection("search").addParameter("mag", "10").endSection();
        assert(builder.getSectionCount() == 1);
        
        std::cout << "✓ Clear resets builder correctly\n";
        std::cout << "Test: Builder Clear... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testSchemaJsonExport() {
    std::cout << "\n=== Test: Schema JSON Export ===\n";
    
    try {
        // Create a schema
        ConfigSchema schema = OopParser::createDefaultSchema();
        
        // Export to JSON Schema
        auto json_schema = schema.toJsonSchema();
        
        // Verify structure
        assert(json_schema.contains("$schema"));
        assert(json_schema.contains("title"));
        assert(json_schema.contains("properties"));
        assert(json_schema.contains("required"));
        
        std::cout << "✓ JSON Schema has correct structure\n";
        
        // Check properties
        auto props = json_schema["properties"];
        assert(props.contains("object"));
        assert(props.contains("search"));
        assert(props.contains("time"));
        
        std::cout << "✓ JSON Schema contains all sections\n";
        
        // Check required sections
        auto required = json_schema["required"];
        bool has_object = false;
        bool has_search = false;
        for (const auto& req : required) {
            if (req == "object") has_object = true;
            if (req == "search") has_search = true;
        }
        assert(has_object && has_search);
        std::cout << "✓ JSON Schema marks required sections\n";
        
        // Print schema
        std::cout << "\nJSON Schema (excerpt):\n";
        std::cout << json_schema.dump(2).substr(0, 500) << "...\n";
        
        std::cout << "\nTest: Schema JSON Export... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testSchemaSaveToFile() {
    std::cout << "\n=== Test: Schema Save to File ===\n";
    
    try {
        ConfigSchema schema = OopParser::createDefaultSchema();
        
        // Save to file
        std::string filepath = "/tmp/ioc_schema.json";
        bool success = schema.saveJsonSchema(filepath);
        assert(success);
        
        // Verify file exists and has content
        std::ifstream file(filepath);
        assert(file.is_open());
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        assert(!content.empty());
        assert(content.find("$schema") != std::string::npos);
        assert(content.find("properties") != std::string::npos);
        
        file.close();
        
        std::cout << "✓ Schema saved to " << filepath << "\n";
        std::cout << "✓ File contains valid JSON Schema\n";
        
        // Cleanup
        std::remove(filepath.c_str());
        
        std::cout << "Test: Schema Save to File... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testSchemaWithConstraints() {
    std::cout << "\n=== Test: Schema JSON with Constraints ===\n";
    
    try {
        // Create custom schema with constraints
        ConfigSchema schema;
        schema.name = "Test Schema";
        schema.version = "1.0";
        
        SectionSpec search_section;
        search_section.name = "search";
        search_section.required = true;
        
        ParameterSpec mag_param;
        mag_param.key = "max_magnitude";
        mag_param.required = false;
        mag_param.description = "Maximum magnitude";
        mag_param.constraint.parseExpression("d <= 20");
        search_section.addParameter(mag_param);
        
        schema.addSection(search_section);
        
        // Export to JSON Schema
        auto json_schema = schema.toJsonSchema();
        
        // Verify constraint in schema
        auto search_props = json_schema["properties"]["search"]["properties"];
        assert(search_props.contains("max_magnitude"));
        
        auto mag_prop = search_props["max_magnitude"];
        assert(mag_prop["type"] == "number");
        assert(mag_prop.contains("maximum"));
        assert(mag_prop["maximum"] == 20);
        
        std::cout << "✓ Constraints exported to JSON Schema correctly\n";
        std::cout << "\nConstraint property:\n";
        std::cout << mag_prop.dump(2) << "\n";
        
        std::cout << "\nTest: Schema with Constraints... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

void testBuilderFromParser() {
    std::cout << "\n=== Test: Builder from Existing Parser ===\n";
    
    try {
        // Create initial parser
        ConfigBuilder initial;
        initial.addSection("object")
               .addParameter("id", "999")
               .addParameter("name", "Test");
        
        auto parser = initial.build();
        
        // Create new builder and copy section from parser
        ConfigBuilder builder2;
        builder2.addSectionFrom(*parser, "object")
                .addSection("search")
                .addParameter("max_magnitude", "15");
        
        auto parser2 = builder2.build();
        
        // Verify
        auto obj_sec = parser2->getSection("object");
        assert(obj_sec != nullptr);
        assert(obj_sec->getParameter("id")->asString() == "999");
        
        auto search_sec = parser2->getSection("search");
        assert(search_sec != nullptr);
        
        std::cout << "✓ Successfully copied section from parser\n";
        std::cout << "✓ New parser has both sections\n";
        
        std::cout << "Test: Builder from Parser... PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        assert(false);
    }
}

int main() {
    std::cout << "\nIOC_Config - ConfigBuilder and Schema Export Tests\n";
    std::cout << "====================================================\n";
    
    try {
        testConfigBuilder();
        testBuilderMethodChaining();
        testBuilderClear();
        testSchemaJsonExport();
        testSchemaSaveToFile();
        testSchemaWithConstraints();
        testBuilderFromParser();
        
        std::cout << "\n====================================================\n";
        std::cout << "All tests completed! ✓\n\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
