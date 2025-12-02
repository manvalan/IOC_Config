# IOC_Config - Advanced Features Guide

## Range Constraints and Parameter Validation

### Overview

IOC_Config now supports advanced parameter validation with range constraints and required/optional fields. This allows you to:

- Mark sections and parameters as required or optional
- Define constraints on numeric values using intuitive expressions
- Create schemas to validate entire configurations
- Support special range notations (e.g., "1..N" for catalog ranges)

## Constraint Expression Syntax

### Range Expressions

#### Inclusive Range: `1..100`
```cpp
RangeConstraint c;
c.parseExpression("1..100");  // Value must be between 1 and 100 (inclusive)
```

**Supported formats:**
- `min..max` - Inclusive range between min and max
- `min..N` - Special notation: from min to catalog size

#### Comparison Operators

**Greater than or equal:**
```cpp
c.parseExpression("d >= 4");   // Value >= 4
```

**Less than or equal:**
```cpp
c.parseExpression("d <= 100");  // Value <= 100
```

**Strictly greater than:**
```cpp
c.parseExpression("d > 5");     // Value > 5
```

**Strictly less than:**
```cpp
c.parseExpression("d < 30");    // Value < 30
```

#### Between Range (exclusive)

```cpp
c.parseExpression("5 < d < 30"); // Value must be between 5 and 30 (exclusive)
```

### Example Constraints

```cpp
// Magnitude must be less than 20
constraint.parseExpression("d < 20");

// Integration step between 0.001 and 10 days
constraint.parseExpression("0.001..10");

// Minimum elevation at least 20 degrees
constraint.parseExpression("d >= 20");

// Catalog range from 1 to all asteroids
constraint.parseExpression("1..N");

// Duration between 0.1 and 1000 seconds (exclusive)
constraint.parseExpression("0.1 < d < 1000");
```

## Working with Schemas

### Create a Schema

```cpp
ConfigSchema schema;
schema.name = "My Configuration Schema";
schema.version = "1.0";

// Add sections
SectionSpec object_section;
object_section.name = "object";
object_section.required = true;
object_section.description = "Target object specification";

// Add parameters to section
ParameterSpec id_param;
id_param.key = "id";
id_param.required = true;
id_param.description = "Object identifier";
object_section.addParameter(id_param);

schema.addSection(object_section);
```

### Default Schema

Use the predefined default schema:

```cpp
ConfigSchema schema = OopParser::createDefaultSchema();

OopParser parser;
parser.setSchema(schema);
```

### Validate Against Schema

```cpp
OopParser parser;
parser.setSchema(schema);

// Load configuration
parser.loadFromJson("config.json");

// Validate
std::vector<std::string> errors;
if (parser.validateWithSchema(schema, errors)) {
    std::cout << "Configuration is valid!\n";
} else {
    for (const auto& error : errors) {
        std::cout << "Error: " << error << "\n";
    }
}
```

## Parameter Specifications

### Required vs Optional Parameters

```cpp
// Required parameter
ParameterSpec required_param;
required_param.key = "id";
required_param.required = true;

// Optional parameter
ParameterSpec optional_param;
optional_param.key = "description";
optional_param.required = false;
optional_param.default_value = "No description";
```

### Parameters with Constraints

```cpp
ParameterSpec magnitude;
magnitude.key = "max_magnitude";
magnitude.required = false;
magnitude.description = "Maximum magnitude for stars";
magnitude.constraint.parseExpression("d < 20");

// Check if value is valid
bool valid = magnitude.isValid("16.5");  // true
bool invalid = magnitude.isValid("25");  // false
```

### Enumerated Parameters

```cpp
ParameterSpec integration_type;
integration_type.key = "type";
integration_type.required = false;
integration_type.allowed_values = {"RK4", "RKF78", "RK45"};

bool valid = integration_type.isValid("RK4");   // true
bool invalid = integration_type.isValid("DP5"); // false
```

## Section Specifications

```cpp
SectionSpec search_section;
search_section.name = "search";
search_section.required = true;
search_section.description = "Occultation search parameters";

// Add parameters
ParameterSpec max_mag;
max_mag.key = "max_magnitude";
max_mag.required = false;
max_mag.constraint.parseExpression("d < 20");
search_section.addParameter(max_mag);

ParameterSpec min_duration;
min_duration.key = "min_duration_sec";
min_duration.required = false;
min_duration.constraint.parseExpression("d > 0");
search_section.addParameter(min_duration);
```

## Complete Example

```cpp
#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    // Create schema with constraints
    ConfigSchema schema;
    schema.name = "Occultation Search";
    schema.version = "1.0";
    
    // Required: object section
    SectionSpec object_spec;
    object_spec.name = "object";
    object_spec.required = true;
    
    ParameterSpec id_spec;
    id_spec.key = "id";
    id_spec.required = true;
    object_spec.addParameter(id_spec);
    
    schema.addSection(object_spec);
    
    // Optional: search section with constraints
    SectionSpec search_spec;
    search_spec.name = "search";
    search_spec.required = false;
    
    ParameterSpec mag_spec;
    mag_spec.key = "max_magnitude";
    mag_spec.required = false;
    mag_spec.constraint.parseExpression("d < 20");
    search_spec.addParameter(mag_spec);
    
    schema.addSection(search_spec);
    
    // Load and validate configuration
    OopParser parser;
    parser.setSchema(schema);
    parser.loadFromJson("config.json");
    
    std::vector<std::string> errors;
    if (parser.validateWithSchema(schema, errors)) {
        std::cout << "✓ Configuration is valid\n";
    } else {
        for (const auto& error : errors) {
            std::cout << "✗ " << error << "\n";
        }
    }
    
    return 0;
}
```

## JSON Schema Integration

Export schema to JSON for external validation:

```cpp
ConfigSchema schema = OopParser::createDefaultSchema();
nlohmann::json schema_json;

// Convert schema to JSON (future feature)
// schema_json = schema.toJson();
```

## Testing

Run the constraints test:

```bash
cd build
./tests/test_constraints
```

## Supported Range Formats Summary

| Format | Example | Meaning |
|--------|---------|---------|
| `min..max` | `1..100` | Between min and max (inclusive) |
| `min..N` | `1..N` | From min to catalog size |
| `d >= val` | `d >= 4` | Greater than or equal |
| `d <= val` | `d <= 100` | Less than or equal |
| `d > val` | `d > 5` | Greater than (exclusive) |
| `d < val` | `d < 30` | Less than (exclusive) |
| `min < d < max` | `5 < d < 30` | Between (both exclusive) |
| `min <= d <= max` | `5 <= d <= 30` | Between (both inclusive) |

## Advanced Validation Features

### Custom Validation Rules

Extend `ParameterSpec` for custom validation:

```cpp
class CustomParameterSpec : public ParameterSpec {
public:
    bool isValid(const std::string& value) const override {
        // Custom validation logic
        return ParameterSpec::isValid(value);
    }
};
```

### Schema Composition

Combine multiple schemas:

```cpp
ConfigSchema base_schema = OopParser::createDefaultSchema();

// Add custom sections
SectionSpec custom_section;
// ... configure ...
base_schema.addSection(custom_section);
```

## Error Messages

Validation provides detailed error messages:

```
Missing required section: object
Missing required parameter 'id' in section 'object'
Parameter 'max_magnitude' in section 'search' failed validation: d < 20
```

## Performance Considerations

- Constraint parsing: O(1) per expression
- Validation: O(n) where n = number of parameters
- Schema storage: O(m) where m = number of sections

## Future Enhancements

- [ ] Support for complex boolean conditions (AND, OR, NOT)
- [ ] Custom validation functions
- [ ] Schema versioning and migration
- [ ] JSON Schema (.json-schema) format export
- [ ] Conditional parameter requirements
