# IOC_Config - OOP File Parser Library

A modern C++ library for parsing, manipulating, and converting OOP (OrbFit-style) configuration files used by IOoccultCalc.

**Author:** Michele Bigi (mikbigi@gmail.com)  
**License:** MIT  
**Version:** 1.2.0

## Features

- **Parse OOP files**: Load and parse IOoccultCalc configuration files
- **Type-safe access**: Strongly-typed access to configuration parameters
- **JSON support**: Convert between OOP and JSON formats
- **Section management**: Organize configurations into logical sections
- **Validation**: Validate configuration files for required parameters
- **Advanced constraints**: Range validation with expressions like "1..100", "d>=4", "5<d<30"
- **Schema-based validation**: Validate configurations against schemas with constraints
- **Fluent builder API**: Build configurations programmatically with method chaining
- **Schema export**: Export validation schemas to JSON Schema format
- **Both static and shared libraries**: Choose between static or dynamic linking
- **Modern C++17**: Written in clean, modern C++

## OOP File Format

OOP files are Fortran-style configuration files with the following structure:

```fortran
! Comments start with exclamation marks
section_name.
        .parameter_key = value
        .another_key = 'string_value'

another_section.
        .numeric_param = 123.45
        .boolean_param = .TRUE.
```

Supported data types:
- **Strings**: `'quoted_strings'` or `"quoted_strings"`
- **Numbers**: `123`, `45.67` (automatically detected as int/float)
- **Booleans**: `.TRUE.`, `.FALSE.`, `true`, `false`, `1`, `0`
- **Arrays**: `['value1', 'value2', 'value3']`

## Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15 or later
- nlohmann/json library

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/manvalan/IOC_Config.git
cd IOC_Config

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build both static and shared libraries
cmake --build .

# Optional: Run tests
ctest

# Optional: Install
sudo cmake --install .
```

### macOS-specific

On macOS, libraries are built as:
- Static: `libioc_config.a`
- Shared: `libioc_config.dylib`

### Linux-specific

On Linux, libraries are built as:
- Static: `libioc_config.a`
- Shared: `libioc_config.so`

### Windows-specific

On Windows, libraries are built as:
- Static: `ioc_config.lib`
- Shared: `ioc_config.dll`

## Usage

### Basic Example

```cpp
#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    // Load OOP file
    OopParser parser("config.oop");
    
    // Access a section
    if (auto section = parser.getSection("object")) {
        if (auto param = section->getParameter("id")) {
            std::cout << "Asteroid ID: " << param->asString() << std::endl;
        }
    }
    
    // Get all sections
    auto sections = parser.getAllSections();
    std::cout << "Total sections: " << sections.size() << std::endl;
    
    return 0;
}
```

### Type Conversion

```cpp
// String conversion
auto str_val = param->asString();

// Numeric conversion
double dbl_val = param->asDouble();
int int_val = param->asInt();

// Boolean conversion
bool bool_val = param->asBoolean();

// Array conversion
auto array_val = param->asStringVector();
```

### JSON Conversion

```cpp
// Convert OOP to JSON
OopParser parser;
parser.loadFromOop("config.oop");
parser.saveToJson("config.json");

// Convert JSON to OOP
parser.loadFromJson("config.json");
parser.saveToOop("config.oop");

// Utility functions
convertOopToJson("input.oop", "output.json");
convertJsonToOop("input.json", "output.oop");
```

### Building Configuration Programmatically

```cpp
OopParser parser;

// Add parameters to sections
parser.setParameter("object", ".id", "'17030'");
parser.setParameter("object", ".name", "'17030 Sierks'");
parser.setParameter("time", ".start_date", "'2025-11-25'");
parser.setParameter("time", ".end_date", "'2025-12-02'");

// Find parameters across sections
if (auto param = parser.findParameter("id")) {
    std::cout << "Found: " << param->asString() << std::endl;
}

// Save
parser.saveToOop("output.oop");
```

### Validation

```cpp
OopParser parser;
parser.loadFromOop("config.oop");

std::vector<std::string> errors;
if (!parser.validate(errors)) {
    for (const auto& error : errors) {
        std::cerr << "Validation error: " << error << std::endl;
    }
}
```

## CMake Integration

To use this library in your CMake project:

```cmake
find_package(IOC_Config REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE IOC_Config::ioc_config)
```

Or link directly:

```cmake
# Link static library
target_link_libraries(my_app PRIVATE ioc_config_static)

# Or link shared library
target_link_libraries(my_app PRIVATE ioc_config_shared)
```

## API Reference

### Main Classes

#### `OopParser`

Main parser class for OOP file manipulation.

**Key Methods:**
- `bool loadFromOop(const std::string& filepath)` - Load OOP file
- `bool saveToOop(const std::string& filepath)` - Save to OOP file
- `bool loadFromJson(const std::string& filepath)` - Load JSON file
- `bool saveToJson(const std::string& filepath)` - Save to JSON file
- `ConfigSectionData* getSection(const std::string& name)` - Get section by name
- `ConfigParameter* findParameter(const std::string& key)` - Find parameter
- `bool setParameter(const std::string& section, const std::string& key, const std::string& value)` - Add/update parameter
- `std::vector<ConfigSectionData> getAllSections()` - Get all sections
- `bool validate(std::vector<std::string>& errors)` - Validate configuration

#### `ConfigSectionData`

Represents a configuration section.

**Key Members:**
- `SectionType type` - Section type enumeration
- `std::string name` - Section name
- `std::map<std::string, ConfigParameter> parameters` - Section parameters

#### `ConfigParameter`

Represents a configuration parameter.

**Key Members:**
- `std::string key` - Parameter key
- `std::string value` - Parameter value
- `std::string type` - Data type

**Key Methods:**
- `std::string asString()` - Get as string
- `double asDouble()` - Get as double
- `int asInt()` - Get as integer
- `bool asBoolean()` - Get as boolean
- `std::vector<std::string> asStringVector()` - Get as vector

#### `ConfigBuilder`

Fluent builder for constructing configurations programmatically.

**Key Methods:**
- `ConfigBuilder& addSection(const std::string& name)` - Add section
- `ConfigBuilder& addParameter(const std::string& key, const std::string& value)` - Add parameter
- `ConfigBuilder& addParameters(const std::map<std::string, std::string>& params)` - Add multiple parameters
- `ConfigBuilder& addSectionFrom(const OopParser& parser, const std::string& name)` - Copy section from parser
- `ConfigBuilder& endSection()` - End current section
- `ConfigBuilder& clear()` - Reset builder
- `OopParser build() const` - Build and return parser
- `size_t getSectionCount() const` - Get number of sections
- `std::vector<std::string> getSectionNames() const` - Get all section names

**Example:**
```cpp
ConfigBuilder builder;
auto parser = builder
    .addSection("object")
    .addParameter("id", "17030")
    .addParameter("name", "Asteroid")
    .endSection()
    .addSection("search")
    .addParameter("max_magnitude", "16.5")
    .endSection()
    .build();  // Returns OopParser
```

#### `ConfigSchema` & `RangeConstraint`

Schema-based validation with range constraints.

**ConfigSchema Methods:**
- `void addSection(const SectionSpec& spec)` - Add section specification
- `bool validateWithSchema(const ConfigSchema& schema, std::vector<std::string>& errors)` - Validate against schema
- `nlohmann::json toJsonSchema() const` - Export to JSON Schema
- `bool saveJsonSchema(const std::string& filepath) const` - Save schema to file

**RangeConstraint Expressions:**
- `"1..100"` - Range: 1 to 100 (inclusive)
- `"1..N"` - Range: 1 to catalog size
- `"d >= 4"` - Greater than or equal to 4
- `"d < 30"` - Less than 30
- `"5 < d < 30"` - Between 5 and 30 (exclusive)

## Examples

See the `examples/` directory for complete examples:

- `basic_usage.cpp` - Basic library usage
- `oop_to_json_converter.cpp` - OOP to JSON conversion tool
- `config_builder_example.cpp` - Building configuration programmatically
- `json_config_example.cpp` - JSON configuration handling
- `builder_example.cpp` - Advanced ConfigBuilder with schema export

Compile and run examples:

```bash
cmake --build . --target example_converter
./examples/example_converter input.oop output.json

cmake --build . --target example_builder
./examples/example_builder
```

## Testing

Run all tests with:

```bash
ctest --output-on-failure
```

Or run individual tests:

```bash
./tests/test_parser           # Core parser tests
./tests/test_builder          # Configuration builder tests
./tests/test_conversion       # OOP ↔ JSON conversion tests
./tests/test_json             # JSON functionality tests
./tests/test_constraints      # Range constraints and schema validation tests
./tests/test_builder_schema   # ConfigBuilder and JSON Schema export tests
```

Test Coverage:
- ✅ OOP file parsing and writing
- ✅ JSON conversion (bidirectional)
- ✅ Range constraint expressions (14/15 tests passing)
- ✅ Schema validation
- ✅ ConfigBuilder fluent API
- ✅ JSON Schema export
- ✅ Parameter and section specifications

## Build Options

- `BUILD_EXAMPLES` (default: ON) - Build example programs
- `BUILD_TESTS` (default: ON) - Build test programs
- `CMAKE_BUILD_TYPE` - Debug or Release build

## Supported Sections

The library recognizes the following standard sections:

| Section | Description |
|---------|-------------|
| `object` | Asteroid/object information |
| `propag` | Propagation settings |
| `asteroids` | Asteroid selection |
| `time` | Time configuration |
| `search` | Search parameters |
| `database` | Database configuration |
| `gaia` | GAIA catalog settings |
| `observer` | Observer location |
| `output` | Output configuration |
| `performance` | Performance settings |
| `occultation` | Occultation parameters |
| `filters` | Filter settings |

## Error Handling

All methods that can fail return a boolean or check the `getLastError()` method:

```cpp
OopParser parser;
if (!parser.loadFromOop("config.oop")) {
    std::cerr << "Error: " << parser.getLastError() << std::endl;
}
```

## Dependencies

- **nlohmann/json** (required) - For JSON serialization

## License

See LICENSE file for details.

## Version

Current version: **1.0.0**

Get version programmatically:

```cpp
std::cout << ioc_config::getLibraryVersion() << std::endl;
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch
3. Write tests for new functionality
4. Submit a pull request

## Support

For issues and questions, please open an issue on the GitHub repository.

## Acknowledgments

- IOccultCalc project
- nlohmann/json library
