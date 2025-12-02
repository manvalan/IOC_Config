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

# Find yaml-cpp (optional, for YAML support)
find_package(yaml-cpp CONFIG QUIET)
if(NOT yaml-cpp_FOUND)
    message(STATUS "yaml-cpp not found via find_package, trying pkg-config...")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(yaml-cpp yaml-cpp)
    endif()
endif()

if(yaml-cpp_FOUND)
    message(STATUS "yaml-cpp found, YAML support will be enabled")
else()
    message(WARNING "yaml-cpp not found, YAML support will be disabled")
    message(STATUS "To install yaml-cpp: brew install yaml-cpp (macOS) or apt-get install libyaml-cpp-dev (Linux)")
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
if(yaml-cpp_FOUND)
    target_link_libraries(ioc_config_static PUBLIC yaml-cpp::yaml-cpp)
    target_compile_definitions(ioc_config_static PRIVATE IOC_CONFIG_YAML_SUPPORT)
endif()
set_target_properties(ioc_config_static PROPERTIES OUTPUT_NAME ioc_config)

# Create shared library
add_library(ioc_config_shared SHARED ${SOURCES})
target_link_libraries(ioc_config_shared PUBLIC nlohmann_json::nlohmann_json)
if(yaml-cpp_FOUND)
    target_link_libraries(ioc_config_shared PUBLIC yaml-cpp::yaml-cpp)
    target_compile_definitions(ioc_config_shared PRIVATE IOC_CONFIG_YAML_SUPPORT)
endif()
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
