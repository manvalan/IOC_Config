cmake_minimum_required(VERSION 3.15)
project(IOC_Config VERSION 1.2.0 
        DESCRIPTION "OOP File Parser Library for IOoccultCalc"
        HOMEPAGE_URL "https://github.com/manvalan/IOC_Config"
        LANGUAGES CXX)

# Project metadata
set(PROJECT_AUTHOR "Michele Bigi")
set(PROJECT_AUTHOR_EMAIL "mikbigi@gmail.com")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler flags for better warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Include FetchContent for automatic dependency management
include(FetchContent)

# --- YAML-CPP Support (Mandatory) ---
find_package(yaml-cpp CONFIG QUIET)
if(NOT yaml-cpp_FOUND)
    message(STATUS "yaml-cpp not found via find_package, fetching from GitHub...")
    FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        0.8.0
    )
    FetchContent_MakeAvailable(yaml-cpp)
    message(STATUS "yaml-cpp fetched and ready.")
else()
    message(STATUS "yaml-cpp found on system.")
endif()

# --- TOML11 Support (Mandatory) ---
find_package(toml11 CONFIG QUIET)
if(NOT toml11_FOUND)
    message(STATUS "toml11 not found via find_package, fetching from GitHub...")
    FetchContent_Declare(
        toml11
        GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
        GIT_TAG        v3.7.1
    )
    FetchContent_MakeAvailable(toml11)
    message(STATUS "toml11 fetched and ready.")
else()
    message(STATUS "toml11 found on system.")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/oop_parser.cpp
)

# Create static library
add_library(ioc_config_static STATIC ${SOURCES})
target_link_libraries(ioc_config_static PUBLIC nlohmann_json::nlohmann_json)
if(TARGET yaml-cpp::yaml-cpp)
    target_link_libraries(ioc_config_static PUBLIC yaml-cpp::yaml-cpp)
else()
    target_link_libraries(ioc_config_static PUBLIC yaml-cpp)
endif()
if(TARGET toml11::toml11)
    target_link_libraries(ioc_config_static PUBLIC toml11::toml11)
else()
    # If built via FetchContent (header only) it might not export the namespace target appropriately depending on version
    # But usually toml11 does. Let's assume toml11_dep if FetchContent created it, or manually add include dir
    # To be safe with FetchContent of toml11, we often just need the include path as it is header-only
    target_link_libraries(ioc_config_static PUBLIC toml11::toml11)
endif()

set_target_properties(ioc_config_static PROPERTIES OUTPUT_NAME ioc_config)

# Create shared library
add_library(ioc_config_shared SHARED ${SOURCES})
target_link_libraries(ioc_config_shared PUBLIC nlohmann_json::nlohmann_json)
if(TARGET yaml-cpp::yaml-cpp)
    target_link_libraries(ioc_config_shared PUBLIC yaml-cpp::yaml-cpp)
else()
    target_link_libraries(ioc_config_shared PUBLIC yaml-cpp)
endif()
target_link_libraries(ioc_config_shared PUBLIC toml11::toml11)

set_target_properties(ioc_config_shared PROPERTIES OUTPUT_NAME ioc_config)

# Add version info
set_target_properties(ioc_config_static ioc_config_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
        DESTINATION include)

# Install libraries
install(TARGETS ioc_config_static ioc_config_shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Generate pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ioc_config.pc.in 
               ${CMAKE_CURRENT_BINARY_DIR}/ioc_config.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ioc_config.pc 
        DESTINATION lib/pkgconfig)

# Enable testing
enable_testing()

# Build CLI tool (optional)
option(BUILD_CLI "Build CLI tool" ON)
if(BUILD_CLI)
    add_subdirectory(tools)
endif()

# Build examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tests (optional)
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

