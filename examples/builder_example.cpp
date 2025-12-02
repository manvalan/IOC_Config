#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

/**
 * @brief Example: Building configurations with fluent API
 * 
 * This example demonstrates the ConfigBuilder fluent interface
 * for constructing configurations programmatically.
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "IOC_Config - ConfigBuilder Example\n";
    std::cout << "========================================\n\n";

    try {
        // ========== Example 1: Basic Configuration ==========
        std::cout << "1. Building a basic configuration...\n";
        std::cout << "   (Using fluent builder API)\n\n";

        ConfigBuilder builder;
        
        auto parser = builder
            // Object section
            .addSection("object")
            .addParameter("id", "17030")
            .addParameter("name", "Asteroid 17030")
            .addParameter("type", "asteroid")
            .endSection()
            
            // Time section
            .addSection("time")
            .addParameter("start_date", "2025-12-01")
            .addParameter("end_date", "2025-12-31")
            .endSection()
            
            // Search section
            .addSection("search")
            .addParameter("max_magnitude", "16.5")
            .addParameter("min_duration_sec", "0.1")
            .endSection()
            
            .build();  // Build and get parser

        std::cout << "✓ Configuration built successfully\n";
        std::cout << "✓ Sections created: " << builder.getSectionCount() << "\n";
        
        auto names = builder.getSectionNames();
        std::cout << "✓ Section names: ";
        for (const auto& name : names) {
            std::cout << name << " ";
        }
        std::cout << "\n\n";

        // ========== Example 2: Save to OOP format ==========
        std::cout << "2. Saving to OOP format...\n";
        std::string oop_file = "/tmp/config_built.oop";
        
        if (parser->saveToOop(oop_file)) {
            std::cout << "✓ Saved to: " << oop_file << "\n\n";
        }

        // ========== Example 3: Convert to JSON ==========
        std::cout << "3. Converting to JSON...\n";
        std::string json_str = parser->toJsonString();
        std::cout << "✓ JSON Output:\n";
        std::cout << json_str << "\n\n";

        // ========== Example 4: Schema Validation ==========
        std::cout << "4. Schema-based validation...\n";
        ConfigSchema schema = OopParser::createDefaultSchema();
        
        std::vector<std::string> errors;
        if (parser->validateWithSchema(schema, errors)) {
            std::cout << "✓ Configuration is valid against default schema\n\n";
        } else {
            std::cout << "✗ Validation errors:\n";
            for (const auto& error : errors) {
                std::cout << "  - " << error << "\n";
            }
            std::cout << "\n";
        }

        // ========== Example 5: Export Schema to JSON Schema ==========
        std::cout << "5. Exporting schema to JSON Schema format...\n";
        
        std::string schema_file = "/tmp/config_schema.json";
        if (schema.saveJsonSchema(schema_file)) {
            std::cout << "✓ Schema saved to: " << schema_file << "\n";
        }
        
        std::string schema_json = schema.toJsonSchemaString();
        std::cout << "✓ Schema JSON (first 300 chars):\n";
        std::cout << schema_json.substr(0, 300) << "...\n\n";

        // ========== Example 6: Chaining with Parameters Map ==========
        std::cout << "6. Adding multiple parameters at once...\n";
        
        ConfigBuilder builder2;
        std::map<std::string, std::string> propag_params = {
            {"type", "RK4"},
            {"step_size", "0.05"},
            {"accuracy", "1e-8"}
        };
        
        builder2.addSection("propag")
                .addParameters(propag_params)
                .endSection()
                .addSection("observer")
                .addParameter("longitude", "-73.935242")
                .addParameter("latitude", "40.728175")
                .endSection();
        
        auto parser2 = builder2.build();
        std::cout << "✓ Created " << builder2.getSectionCount() << " sections\n";
        std::cout << "✓ Propagation parameters: " << propag_params.size() << "\n\n";

        // ========== Example 7: Copying sections from existing parser ==========
        std::cout << "7. Copying sections between parsers...\n";
        
        ConfigBuilder builder3;
        builder3.addSectionFrom(*parser, "object")  // Copy from first parser
                .addSection("database")
                .addParameter("type", "sqlite")
                .addParameter("path", "/path/to/asteroids.db");
        
        auto parser3 = builder3.build();
        std::cout << "✓ Copied 'object' section from first parser\n";
        std::cout << "✓ New parser has " << builder3.getSectionCount() << " sections\n\n";

        // ========== Example 8: Method Chaining Validation ==========
        std::cout << "8. Method chaining return values...\n";
        ConfigBuilder builder4;
        
        ConfigBuilder& ref1 = builder4.addSection("test");
        ConfigBuilder& ref2 = ref1.addParameter("key", "value");
        ConfigBuilder& ref3 = ref2.endSection();
        
        // All references point to the same builder
        if (&ref1 == &builder4 && &ref2 == &builder4 && &ref3 == &builder4) {
            std::cout << "✓ Method chaining returns builder reference\n";
            std::cout << "✓ All methods return same object for chaining\n\n";
        }

        // ========== Example 9: Clear and Reuse ==========
        std::cout << "9. Clearing and reusing builder...\n";
        
        std::cout << "   Sections before clear: " << builder4.getSectionCount() << "\n";
        builder4.clear();
        std::cout << "   Sections after clear: " << builder4.getSectionCount() << "\n";
        
        builder4.addSection("new_section")
                .addParameter("new_key", "new_value");
        
        std::cout << "   Sections after adding: " << builder4.getSectionCount() << "\n\n";

        std::cout << "========================================\n";
        std::cout << "All examples completed successfully! ✓\n";
        std::cout << "========================================\n\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
