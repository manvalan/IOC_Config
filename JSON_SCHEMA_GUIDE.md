# JSON Schema Export Guide

## Overview

IOC_Config can export validation schemas to JSON Schema format, enabling integration with external tools and validators. This allows you to:

- Validate configurations with external JSON Schema validators
- Generate documentation from schemas
- Integrate with IDE schema validators
- Share validation rules with other systems

## Quick Start

```cpp
#include "ioc_config/oop_parser.h"
using namespace ioc_config;

// Create or load a schema
ConfigSchema schema = OopParser::createDefaultSchema();

// Export to JSON Schema file
schema.saveJsonSchema("config_schema.json");

// Or get as JSON object
nlohmann::json json_schema = schema.toJsonSchema();

// Or get as formatted string
std::string schema_str = schema.toJsonSchemaString();
```

## API Reference

### ConfigSchema Methods

```cpp
// Export to JSON object
nlohmann::json toJsonSchema() const;

// Save to file
bool saveJsonSchema(const std::string& filepath) const;

// Get as formatted string
std::string toJsonSchemaString(int indent = 2) const;
```

## Generated JSON Schema Structure

### Basic Example

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "IOoccultCalc Default",
  "version": "1.0",
  "type": "object",
  "properties": {
    "object": {
      "type": "object",
      "description": "Target object specification",
      "properties": {
        "id": {
          "type": "string",
          "description": "Object identifier"
        },
        "name": {
          "type": "string",
          "description": "Object name"
        }
      },
      "required": ["id", "name"]
    },
    "search": {
      "type": "object",
      "description": "Occultation search parameters",
      "properties": {
        "max_magnitude": {
          "type": "number",
          "description": "Maximum magnitude",
          "exclusiveMaximum": 20,
          "constraint": "d < 20"
        }
      }
    }
  },
  "required": ["object", "search", "time"]
}
```

## Features

### 1. Required/Optional Sections

Sections marked as required appear in the schema's `required` array:

```cpp
SectionSpec object_section;
object_section.required = true;  // Will be in schema's required array
schema.addSection(object_section);

SectionSpec optional_section;
optional_section.required = false;  // Will NOT be in required array
schema.addSection(optional_section);
```

Generated JSON Schema:
```json
{
  "required": ["object", "time", "search"]
}
```

### 2. Parameter Descriptions

All parameter descriptions are included:

```cpp
ParameterSpec param;
param.key = "max_magnitude";
param.description = "Maximum magnitude for stars";
param.required = false;
```

Generated JSON Schema:
```json
{
  "max_magnitude": {
    "type": "number",
    "description": "Maximum magnitude for stars"
  }
}
```

### 3. Range Constraints

Range constraints are exported as JSON Schema constraints:

```cpp
ParameterSpec param;
param.key = "magnitude";
param.constraint.parseExpression("d < 20");  // Less than 20
```

Generated JSON Schema:
```json
{
  "magnitude": {
    "type": "number",
    "exclusiveMaximum": 20,
    "constraint": "d < 20"
  }
}
```

### Supported Constraint Types

| Constraint | JSON Schema | Example |
|-----------|-----------|---------|
| `d >= val` | `minimum` | `"d >= 4"` → `"minimum": 4` |
| `d <= val` | `maximum` | `"d <= 100"` → `"maximum": 100` |
| `d > val` | `exclusiveMinimum` | `"d > 5"` → `"exclusiveMinimum": 5` |
| `d < val` | `exclusiveMaximum` | `"d < 30"` → `"exclusiveMaximum": 30` |
| `min..max` | `minimum`/`maximum` | `"1..100"` → `"minimum": 1, "maximum": 100` |

### 4. Enumerated Values

Enum constraints are exported as `enum`:

```cpp
ParameterSpec integration_type;
integration_type.key = "type";
integration_type.allowed_values = {"RK4", "RKF78", "RK45"};
```

Generated JSON Schema:
```json
{
  "type": {
    "enum": ["RK4", "RKF78", "RK45"]
  }
}
```

## Usage Examples

### Example 1: Export Default Schema

```cpp
#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    // Get default schema
    auto schema = OopParser::createDefaultSchema();
    
    // Save to file
    if (schema.saveJsonSchema("ioc_schema.json")) {
        std::cout << "✓ Schema exported to ioc_schema.json\n";
    }
    
    return 0;
}
```

### Example 2: Create Custom Schema and Export

```cpp
ConfigSchema schema;
schema.name = "My Custom Config";
schema.version = "1.0";

// Add object section
SectionSpec object_section;
object_section.name = "object";
object_section.required = true;
object_section.description = "Object information";

ParameterSpec id_param;
id_param.key = "id";
id_param.required = true;
id_param.description = "Object ID";
object_section.addParameter(id_param);

schema.addSection(object_section);

// Export
schema.saveJsonSchema("custom_schema.json");

// Or print
std::cout << schema.toJsonSchemaString() << "\n";
```

### Example 3: Use with Configuration Validation

```cpp
// Create configuration
auto parser = ConfigBuilder()
    .addSection("object")
    .addParameter("id", "17030")
    .addParameter("name", "Test")
    .endSection()
    .build();

// Get schema
ConfigSchema schema = OopParser::createDefaultSchema();

// Validate
std::vector<std::string> errors;
if (parser.validateWithSchema(schema, errors)) {
    std::cout << "✓ Valid\n";
} else {
    for (const auto& error : errors) {
        std::cout << "✗ " << error << "\n";
    }
}

// Export schema for external validators
schema.saveJsonSchema("schema.json");
```

### Example 4: Generate Documentation

```cpp
ConfigSchema schema = OopParser::createDefaultSchema();
nlohmann::json json_schema = schema.toJsonSchema();

std::cout << "# Configuration Schema\n\n";
std::cout << "**Title:** " << json_schema["title"] << "\n";
std::cout << "**Version:** " << json_schema["version"] << "\n\n";

std::cout << "## Required Sections\n";
for (const auto& req : json_schema["required"]) {
    std::cout << "- " << req << "\n";
}

std::cout << "\n## Properties\n";
for (auto& [name, prop] : json_schema["properties"].items()) {
    std::cout << "\n### " << name << "\n";
    std::cout << prop["description"] << "\n";
}
```

### Example 5: Integration with External Validators

```cpp
// Export schema
ConfigSchema schema = OopParser::createDefaultSchema();
schema.saveJsonSchema("config_schema.json");

// Now you can validate with external tools:
// - VSCode with JSON Schema extension
// - JSON Schema validators
// - Documentation generators
// - API validators

// The schema is a valid JSON Schema Draft 7
```

## JSON Schema Validation Tools

You can use the exported schema with:

### 1. VSCode

Add to `.vscode/settings.json`:
```json
{
  "json.schemas": [
    {
      "fileMatch": ["config.json"],
      "url": "file:///path/to/config_schema.json"
    }
  ]
}
```

### 2. Command-line Validators

```bash
# Using ajv
npm install -g ajv-cli
ajv validate -s config_schema.json -d config.json

# Using tv4
npm install -g tv4
tv4 validate config.json config_schema.json
```

### 3. Python

```python
import json
import jsonschema

with open('config_schema.json') as f:
    schema = json.load(f)

with open('config.json') as f:
    config = json.load(f)

try:
    jsonschema.validate(config, schema)
    print("✓ Configuration is valid")
except jsonschema.ValidationError as e:
    print(f"✗ {e.message}")
```

## Schema Structure

### Root Properties

| Property | Description |
|----------|-------------|
| `$schema` | JSON Schema version (draft-07) |
| `title` | Schema name |
| `version` | Schema version |
| `type` | Always "object" |
| `properties` | Section definitions |
| `required` | Required section names |

### Section Properties

| Property | Description |
|----------|-------------|
| `type` | Always "object" |
| `description` | Section description |
| `properties` | Parameter definitions |
| `required` | Required parameters |

### Parameter Properties

| Property | Description |
|----------|-------------|
| `type` | "string", "number", "boolean" |
| `description` | Parameter description |
| `enum` | Allowed values (if enumerated) |
| `minimum` | Minimum value (if constrained) |
| `maximum` | Maximum value (if constrained) |
| `exclusiveMinimum` | Exclusive minimum |
| `exclusiveMaximum` | Exclusive maximum |
| `constraint` | Original constraint expression |

## Best Practices

1. **Always include descriptions** - They appear in the schema and documentation
2. **Mark required sections/parameters** - Helps external validators
3. **Use meaningful names** - Improves readability
4. **Export regularly** - Keep schema synchronized with code
5. **Validate configurations** - Both programmatically and with external tools

## Examples

See `examples/builder_example.cpp` for:
- Creating custom schemas
- Exporting to files
- Using with validators

## See Also

- [JSON Schema Documentation](https://json-schema.org/)
- [JSON Schema Draft 7](https://json-schema.org/draft-07/json-schema-core.html)
- `ADVANCED_FEATURES.md` - Constraint expression syntax
- `BUILDER_GUIDE.md` - Building configurations
