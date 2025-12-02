cmake_minimum_required(VERSION 3.15)

# CLI Tool: ioc-config
add_executable(ioc-config ioc_config_cli.cpp)
target_link_libraries(ioc-config PRIVATE ioc_config_static)
target_include_directories(ioc-config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Set install target
install(TARGETS ioc-config DESTINATION bin)
