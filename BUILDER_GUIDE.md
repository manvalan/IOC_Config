# ConfigBuilder Fluent API Guide

## Overview

`ConfigBuilder` is a fluent builder class that allows you to construct OOP configurations programmatically using a convenient method-chaining API.

## Quick Start

```cpp
#include "ioc_config/oop_parser.h"
using namespace ioc_config;

// Build a configuration with fluent API
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "17030")
    .addParameter("name", "Asteroid 17030")
    .endSection()
    .addSection("search")
    .addParameter("max_magnitude", "16.5")
    .endSection()
    .build();  // Returns OopParser
```

## API Reference

### Method Chaining

All builder methods return `ConfigBuilder&` to enable method chaining:

```cpp
ConfigBuilder& addSection(const std::string& name);
ConfigBuilder& addParameter(const std::string& key, const std::string& value);
ConfigBuilder& addParameters(const std::map<std::string, std::string>& params);
ConfigBuilder& endSection();
ConfigBuilder& addSectionFrom(const OopParser& parser, const std::string& name);
ConfigBuilder& clear();
```

### Builder Information

```cpp
OopParser build() const;                          // Build and return parser
OopParser getParser() const;                      // Get without resetting
size_t getSectionCount() const;                   // Number of sections
std::vector<std::string> getSectionNames() const; // All section names
```

## Usage Patterns

### Pattern 1: Simple Configuration

```cpp
ConfigBuilder builder;
builder.addSection("object")
       .addParameter("id", "123")
       .addParameter("name", "Comet");

auto parser = builder.build();
```

### Pattern 2: Multiple Sections

```cpp
ConfigBuilder builder;

builder.addSection("object")
       .addParameter("id", "999")
       .addParameter("type", "asteroid")
       .endSection()
       .addSection("time")
       .addParameter("start_date", "2025-12-01")
       .addParameter("end_date", "2025-12-31")
       .endSection();

auto parser = builder.build();
```

### Pattern 3: Add Multiple Parameters

```cpp
std::map<std::string, std::string> propag_params = {
    {"type", "RK4"},
    {"step_size", "0.05"},
    {"accuracy", "1e-8"}
};

auto parser = ConfigBuilder()
    .addSection("propag")
    .addParameters(propag_params)
    .build();
```

### Pattern 4: Copy from Existing Parser

```cpp
OopParser existing_parser;
existing_parser.loadFromOop("config.oop");

auto parser = ConfigBuilder()
    .addSectionFrom(existing_parser, "object")  // Copy "object" section
    .addSection("search")
    .addParameter("max_magnitude", "15")
    .build();
```

### Pattern 5: Build and Save

```cpp
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "17030")
    .endSection()
    .build();

// Save to OOP format
parser.saveToOop("output.oop");

// Save to JSON format
parser.saveToJson("output.json");

// Get JSON string
std::string json_str = parser.toJsonString();
```

### Pattern 6: Reuse and Reset

```cpp
ConfigBuilder builder;

// Build first configuration
builder.addSection("object").addParameter("id", "1");
auto parser1 = builder.build();

// Reset and build another
builder.clear();
builder.addSection("time").addParameter("start_date", "2025-01-01");
auto parser2 = builder.build();
```

## Advanced Examples

### Build with Validation

```cpp
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "17030")
    .addParameter("name", "Test")
    .endSection()
    .addSection("search")
    .addParameter("max_magnitude", "16.5")
    .endSection()
    .build();

// Validate against schema
ConfigSchema schema = OopParser::createDefaultSchema();
std::vector<std::string> errors;

if (parser.validateWithSchema(schema, errors)) {
    std::cout << "✓ Configuration is valid\n";
} else {
    for (const auto& error : errors) {
        std::cout << "✗ " << error << "\n";
    }
}
```

### Build, Validate, and Export

```cpp
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "999")
    .addParameter("name", "Asteroid")
    .endSection()
    .addSection("time")
    .addParameter("start_date", "2025-12-01")
    .addParameter("end_date", "2025-12-31")
    .endSection()
    .build();

// Export schema to JSON Schema format
ConfigSchema schema = OopParser::createDefaultSchema();
schema.saveJsonSchema("schema.json");

// Get JSON string for validation tools
std::string schema_json = schema.toJsonSchemaString();
```

### Conditional Building

```cpp
ConfigBuilder builder;

builder.addSection("object")
       .addParameter("id", "17030")
       .addParameter("name", "Test");

if (include_search) {
    builder.addSection("search")
           .addParameter("max_magnitude", "16.5");
}

auto parser = builder.build();
```

### Building from User Input

```cpp
ConfigBuilder builder;
std::string section_name, key, value;

while (std::getline(std::cin, section_name)) {
    if (section_name == "DONE") break;
    
    builder.addSection(section_name);
    
    while (std::getline(std::cin, key) && key != "") {
        std::getline(std::cin, value);
        builder.addParameter(key, value);
    }
}

auto parser = builder.build();
parser.saveToOop("user_config.oop");
```

## Key Features

### 1. Fluent Interface
All methods return a reference to the builder for chaining:
```cpp
builder
    .addSection("s1")
    .addParameter("p1", "v1")
    .addParameter("p2", "v2")
    .endSection()
    .addSection("s2")
    .addParameter("p3", "v3");
```

### 2. Type Safety
Parameters are stored as strings but can be accessed with type conversion:
```cpp
auto param = parser.findParameter("p1");
double value = param->asDouble();
int count = param->asInt();
bool flag = param->asBoolean();
```

### 3. Automatic Type Detection
The builder automatically detects and stores data types:
```cpp
builder.addParameter("count", "42");        // Detected as int
builder.addParameter("ratio", "3.14");      // Detected as float
builder.addParameter("name", "Test");       // Detected as string
builder.addParameter("enabled", ".TRUE.");  // Detected as bool
```

### 4. Reusability
Build configurations and reuse them:
```cpp
auto parser1 = builder.build();
auto parser2 = builder.build();  // Create another copy
parser1.saveToOop("config1.oop");
parser2.saveToOop("config2.oop");
```

### 5. Section Management
```cpp
// Get section names
auto names = builder.getSectionNames();
for (const auto& name : names) {
    std::cout << name << "\n";
}

// Get section count
size_t count = builder.getSectionCount();
std::cout << "Sections: " << count << "\n";
```

## Error Handling

The builder throws exceptions if:
1. You add parameters before adding a section
2. You try to copy a section that doesn't exist

```cpp
try {
    builder.addParameter("key", "value");  // Error: no section!
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
}
```

## Comparison: Manual vs Builder

### Manual Construction
```cpp
OopParser parser;
parser.setParameter("object", "id", "17030");
parser.setParameter("object", "name", "Test");
parser.setParameter("search", "max_magnitude", "16.5");
```

### Builder Pattern
```cpp
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "17030")
    .addParameter("name", "Test")
    .endSection()
    .addSection("search")
    .addParameter("max_magnitude", "16.5")
    .build();
```

The builder pattern is:
- More readable
- Less error-prone
- More efficient (groups operations)
- More maintainable

## Best Practices

1. **Use method chaining** for concise code
2. **Call endSection()** for clarity (optional but recommended)
3. **Reuse builders** for similar configurations
4. **Validate after building** with schemas
5. **Handle exceptions** when copying sections

## See Also

- `examples/builder_example.cpp` - Comprehensive examples
- `ADVANCED_FEATURES.md` - Validation and constraints
- `QUICKSTART.md` - Getting started guide
