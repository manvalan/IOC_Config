cmake_minimum_required(VERSION 3.15)

# Test 1: Parser functionality
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE ioc_config_static)
target_include_directories(test_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME ParserTest COMMAND test_parser)

# Test 2: Config builder
add_executable(test_builder test_builder.cpp)
target_link_libraries(test_builder PRIVATE ioc_config_static)
target_include_directories(test_builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME BuilderTest COMMAND test_builder)

# Test 3: JSON conversion
add_executable(test_conversion test_conversion.cpp)
target_link_libraries(test_conversion PRIVATE ioc_config_static)
target_include_directories(test_conversion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME ConversionTest COMMAND test_conversion)

# Test 4: JSON functionality
add_executable(test_json test_json.cpp)
target_link_libraries(test_json PRIVATE ioc_config_static)
target_include_directories(test_json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME JsonTest COMMAND test_json)

# Test 5: Constraints and advanced validation
add_executable(test_constraints test_constraints.cpp)
target_link_libraries(test_constraints PRIVATE ioc_config_static)
target_include_directories(test_constraints PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME ConstraintsTest COMMAND test_constraints)

# Test 6: ConfigBuilder and Schema JSON Export
add_executable(test_builder_schema test_builder_schema.cpp)
target_link_libraries(test_builder_schema PRIVATE ioc_config_static)
target_include_directories(test_builder_schema PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME BuilderSchemaTest COMMAND test_builder_schema)

# Test 7: YAML functionality
add_executable(test_yaml test_yaml.cpp)
target_link_libraries(test_yaml PRIVATE ioc_config_static)
target_include_directories(test_yaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_test(NAME YamlTest COMMAND test_yaml)

