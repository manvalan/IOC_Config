# IOC_Config v1.1.0 - JSON-Native Configuration Support

## Release Overview

Version 1.1.0 introduces comprehensive native JSON configuration support, allowing seamless bidirectional conversion between JSON and OOP formats. Configuration can now be loaded, manipulated, and saved entirely in JSON format.

## New Features

### 1. Native JSON Support

The parser now has full native JSON support without needing OOP format as an intermediate:

```cpp
// Load from JSON string
OopParser parser;
parser.loadFromJsonString(jsonString);

// Get as JSON string
std::string jsonOutput = parser.toJsonString(2);  // Pretty-printed

// Load from JSON object
nlohmann::json config;
parser.fromJson(config);

// Get as JSON object
nlohmann::json output = parser.toJson();
```

### 2. JSON File Operations

```cpp
// Load JSON configuration
parser.loadFromJson("config.json");

// Save to JSON
parser.saveToJson("output.json");
```

### 3. Type-Safe JSON Handling

Values are automatically converted to appropriate JSON types:
- Strings → JSON strings
- Numbers → JSON numbers (float/int auto-detected)
- Booleans → JSON booleans
- Arrays → JSON arrays

### 4. Automatic Section Creation

Sections are automatically created when using `setParameter()` - no need to pre-create sections:

```cpp
OopParser parser;
parser.setParameter("object", "id", "'asteroid123'");  // Creates "object" section
parser.setParameter("object", "name", "'Test Asteroid'");
// Configuration ready to use
```

### 5. Schema Validation

Validate configurations against JSON schema:

```cpp
nlohmann::json schema = {
    {"required", {"object", "time", "search"}}
};

std::vector<std::string> errors;
if (parser.validateAgainstSchema(schema, errors)) {
    std::cout << "Configuration is valid\n";
}
```

### 6. Programmatic Configuration

Build configurations entirely in code:

```cpp
OopParser parser;
parser.setParameter("object", "id", "'17030'");
parser.setParameter("object", "name", "'Sierks'");
parser.setParameter("time", "start_date", "'2025-11-25'");
parser.setParameter("search", "max_magnitude", "16.0");

// Output in any format
std::string json = parser.toJsonString();
parser.saveToOop("config.oop");
parser.saveToJson("config.json");
```

## API Additions

### New Methods

```cpp
// JSON String Operations
bool loadFromJsonString(const std::string& jsonString);
std::string saveToJsonString() const;
std::string toJsonString(int indent = 2) const;

// JSON Object Operations
bool loadFromJsonObject(const nlohmann::json& jsonObj);
nlohmann::json saveToJsonObject() const;
nlohmann::json toJson() const;
bool fromJson(const nlohmann::json& jsonObj);

// Schema Validation
bool validateAgainstSchema(const nlohmann::json& schemaJson,
                          std::vector<std::string>& errors) const;

// Enhanced setParameter (now auto-creates sections)
bool setParameter(const std::string& sectionName,
                  const std::string& paramKey,
                  const std::string& value);
```

### Modified Methods

- `setParameter()` - Now automatically creates sections if they don't exist

## Examples

### Example 1: Load and Process JSON

```cpp
#include "ioc_config/oop_parser.h"

OopParser parser;
parser.loadFromJson("configuration.json");

// Access sections and parameters
auto obj_section = parser.getSection("object");
if (auto id = obj_section->getParameter("id")) {
    std::string asteroid_id = id->asString();
}

// Get entire configuration as JSON
std::cout << parser.toJsonString(2) << std::endl;
```

### Example 2: Build Configuration Programmatically

```cpp
OopParser parser;

// Build configuration
parser.setParameter("object", "id", "'2025 PA'");
parser.setParameter("object", "name", "'Asteroid 2025 PA'");
parser.setParameter("time", "start_date", "'2025-04-12'");
parser.setParameter("time", "end_date", "'2025-04-15'");
parser.setParameter("search", "max_magnitude", "18.0");

// Output as JSON
std::cout << parser.toJsonString() << std::endl;

// Or save to files
parser.saveToJson("config.json");
parser.saveToOop("config.oop");
```

### Example 3: Bidirectional Conversion

```cpp
// Convert from OOP to JSON
convertOopToJson("input.oop", "output.json");

// Or use objects
OopParser parser;
parser.loadFromOop("input.oop");

// Manipulate
parser.setParameter("object", "modified", "true");

// Save in either format
parser.saveToJson("output.json");
parser.saveToOop("output.oop");
```

## Sample Configuration Files

### JSON Format (config.json)
```json
{
  "object": {
    "id": "17030",
    "name": "17030 Sierks"
  },
  "time": {
    "start_date": "2025-11-25",
    "end_date": "2025-12-02"
  },
  "search": {
    "max_magnitude": 16.0,
    "search_radius_deg": 10.0
  }
}
```

### Equivalent OOP Format (config.oop)
```fortran
object.
        .id = '17030'
        .name = '17030 Sierks'

time.
        .start_date = '2025-11-25'
        .end_date = '2025-12-02'

search.
        .max_magnitude = 16.0
        .search_radius_deg = 10.0
```

## New Examples

- `json_config_example.cpp` - Demonstrates JSON configuration workflows
- `config_template.json` - Complete configuration template in JSON
- `config_schema.json` - JSON Schema for validating configurations

## New Tests

- `test_json.cpp` - Comprehensive JSON functionality tests

## Improvements

1. **Automatic Section Management** - Sections are created on-demand
2. **Better Type Handling** - Automatic type detection and conversion
3. **More Flexible API** - Work with JSON strings, objects, or files
4. **Enhanced Validation** - Schema-based validation support

## Backward Compatibility

✅ **Fully backward compatible**
- All existing OOP parsing features work unchanged
- Existing API methods unchanged
- New methods are additive only

## Performance

- JSON parsing: O(n) where n = number of fields
- JSON generation: O(p) where p = number of parameters
- Type conversion: O(1)

## Dependencies

- nlohmann/json (3.x or higher) - included as before

## Known Limitations

1. Schema validation is basic (required fields only) - can be enhanced
2. Large JSON files (>1GB) may require streaming parser
3. Comments in JSON not supported (JSON spec limitation)

## Migration Guide

For users upgrading from v1.0.0:

### Before (v1.0.0)
```cpp
OopParser parser;
parser.loadFromOop("config.oop");
parser.saveToJson("config.json");  // OOP → JSON conversion

// Section must exist first
if (parser.getSection("object")) {
    parser.setParameter("object", "id", "'123'");
}
```

### After (v1.1.0)
```cpp
OopParser parser;
parser.loadFromJson("config.json");  // Direct JSON support

// Section auto-created
parser.setParameter("object", "id", "'123'");

// Get as JSON easily
std::cout << parser.toJsonString() << std::endl;
```

## Future Enhancements (v1.2.0)

- Streaming JSON parser for large files
- JSON Pointer (RFC 6901) support for accessing nested values
- More advanced schema validation
- Configuration merging/diffing utilities

## Contributors

- Core development team

## Support

For issues, feature requests, or questions:
- GitHub Issues: https://github.com/manvalan/IOC_Config/issues
- Documentation: See README.md and QUICKSTART.md

---

**Release Date:** December 1, 2025  
**Version:** 1.1.0  
**Status:** Stable
