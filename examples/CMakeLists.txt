cmake_minimum_required(VERSION 3.15)

# Example 1: Basic usage
add_executable(example_basic basic_usage.cpp)
target_link_libraries(example_basic PRIVATE ioc_config_static)
target_include_directories(example_basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example 2: OOP to JSON converter
add_executable(example_converter oop_to_json_converter.cpp)
target_link_libraries(example_converter PRIVATE ioc_config_static)
target_include_directories(example_converter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example 3: Config builder
add_executable(example_config_builder config_builder_example.cpp)
target_link_libraries(example_config_builder PRIVATE ioc_config_static)
target_include_directories(example_config_builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example 4: JSON configuration
add_executable(example_json json_config_example.cpp)
target_link_libraries(example_json PRIVATE ioc_config_static)
target_include_directories(example_json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example 5: Advanced builder with schema export
add_executable(example_builder builder_example.cpp)
target_link_libraries(example_builder PRIVATE ioc_config_static)
target_include_directories(example_builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example 6: YAML configuration
add_executable(example_yaml yaml_config_example.cpp)
target_link_libraries(example_yaml PRIVATE ioc_config_static)
target_include_directories(example_yaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

