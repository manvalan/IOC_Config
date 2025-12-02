# IOC_Config - Quick Start Guide

Get up and running with IOC_Config in 5 minutes!

**Author:** Michele Bigi (mikbigi@gmail.com)

## Installation

### 1. Prerequisites

Make sure you have:
- CMake 3.15+
- C++17 compatible compiler
- nlohmann_json (install via package manager or included)

On macOS:
```bash
brew install nlohmann-json cmake
```

On Ubuntu/Debian:
```bash
sudo apt-get install nlohmann-json3-dev cmake
```

### 2. Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

That's it! You now have:
- `libioc_config.a` (static library)
- `libioc_config.dylib` or `.so` (shared library, depending on OS)

### 3. Install (Optional)

```bash
sudo cmake --install .
```

## Quick Example

### Load and parse an OOP file

```cpp
#include "ioc_config/oop_parser.h"
#include <iostream>

using namespace ioc_config;

int main() {
    // Create parser
    OopParser parser;
    
    // Load OOP file
    if (!parser.loadFromOop("config.oop")) {
        std::cerr << "Error: " << parser.getLastError() << std::endl;
        return 1;
    }
    
    // Get object section
    auto section = parser.getSection("object");
    if (section) {
        auto id = section->getParameter("id");
        if (id) {
            std::cout << "Asteroid ID: " << id->asString() << std::endl;
        }
    }
    
    // List all sections
    for (const auto& sec : parser.getAllSections()) {
        std::cout << "Section: " << sec.name << " (" 
                  << sec.parameters.size() << " params)" << std::endl;
    }
    
    return 0;
}
```

### Convert OOP to JSON

```cpp
#include "ioc_config/oop_parser.h"

using namespace ioc_config;

int main() {
    convertOopToJson("config.oop", "config.json");
    return 0;
}
```

### Build configuration programmatically

```cpp
#include "ioc_config/oop_parser.h"

using namespace ioc_config;

int main() {
    OopParser parser;
    
    parser.setParameter("object", ".id", "'17030'");
    parser.setParameter("object", ".name", "'Asteroid 17030'");
    parser.setParameter("time", ".start_date", "'2025-11-25'");
    
    parser.saveToOop("my_config.oop");
    parser.saveToJson("my_config.json");
    
    return 0;
}
```

## Link to Library

In your `CMakeLists.txt`:

```cmake
# Use pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(IOC_CONFIG REQUIRED ioc_config)

add_executable(my_app main.cpp)
target_link_libraries(my_app ${IOC_CONFIG_LIBRARIES})
target_include_directories(my_app PRIVATE ${IOC_CONFIG_INCLUDE_DIRS})
```

Or directly:

```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app ioc_config_static)  # or ioc_config_shared
target_include_directories(my_app PRIVATE /usr/local/include)
```

Or with manual flags:

```bash
g++ -std=c++17 my_app.cpp -o my_app -lioc_config -I/usr/local/include
```

## Run Examples

Build examples:
```bash
cmake --build . --target example_basic
cmake --build . --target example_converter
```

Run converter:
```bash
./examples/example_converter input.oop output.json
```

## Run Tests

```bash
ctest --output-on-failure
```

## Common Tasks

### Read a parameter value

```cpp
OopParser parser("config.oop");

// By section
if (auto section = parser.getSection("object")) {
    if (auto param = section->getParameter("id")) {
        std::string id = param->asString();
    }
}

// Search all sections
if (auto param = parser.findParameter("id")) {
    std::string id = param->asString();
}
```

### Convert types

```cpp
auto param = parser.findParameter("max_magnitude");

std::string s = param->asString();        // "16.0"
double d = param->asDouble();              // 16.0
int i = param->asInt();                    // 16
std::vector<std::string> v = param->asStringVector(); // For arrays
```

### Validate configuration

```cpp
std::vector<std::string> errors;
if (!parser.validate(errors)) {
    for (const auto& error : errors) {
        std::cout << "Error: " << error << std::endl;
    }
}
```

### Get library version

```cpp
std::cout << ioc_config::getLibraryVersion() << std::endl;  // "1.0.0"
```

## Supported Sections

- `object` - Asteroid information
- `propag` - Propagation settings
- `asteroids` - Asteroid selection
- `time` - Time configuration
- `search` - Search parameters
- `database` - Database settings
- `gaia` - GAIA catalog settings
- `observer` - Observer location
- `output` - Output configuration
- `performance` - Performance settings
- `occultation` - Occultation parameters
- `filters` - Filter settings

## File Formats

### OOP Format

```fortran
! Comments start with !
section_name.
        .parameter_key = 'string_value'
        .numeric_key = 123.45
        .boolean_key = .TRUE.
        .array_key = ['val1', 'val2']
```

### JSON Format

```json
{
  "section_name": {
    "parameter_key": "string_value",
    "numeric_key": 123.45,
    "boolean_key": true,
    "array_key": ["val1", "val2"]
  }
}
```

## Troubleshooting

### CMake can't find nlohmann_json

Install with:
```bash
brew install nlohmann-json  # macOS
apt-get install nlohmann-json3-dev  # Ubuntu
```

Or from source: https://github.com/nlohmann/json

### Linker errors

Make sure you're linking the right library:
```bash
# List available libraries
ls -la /usr/local/lib/libioc_config*

# Link explicitly
g++ main.cpp -o app -L/usr/local/lib -lioc_config -I/usr/local/include
```

### File not found errors

Paths are relative to current directory. Use absolute paths or check working directory:
```cpp
OopParser parser("/absolute/path/to/config.oop");
```

## Next Steps

- Read the full [README.md](README.md) for detailed documentation
- Check [examples/](examples/) for more usage patterns
- Run tests in [tests/](tests/) for more examples
