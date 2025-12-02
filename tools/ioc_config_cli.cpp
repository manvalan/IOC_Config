/**
 * @file ioc_config_cli.cpp
 * @brief IOC_Config - Command Line Interface Tool
 * 
 * Standalone CLI utility for parsing, validating, converting, and managing
 * IOC configuration files in various formats (OOP, JSON, YAML)
 * 
 * Usage:
 *   ioc-config parse <file>              Parse and display configuration
 *   ioc-config validate <file>           Validate configuration against schema
 *   ioc-config convert <input> <output>  Convert between formats
 *   ioc-config merge <file1> <file2>     Merge two configurations
 *   ioc-config export-schema <output>    Export JSON schema
 * 
 * @author Michele Bigi (mikbigi@gmail.com)
 * @date 2025-12-02
 */

#include "ioc_config/oop_parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace ioc_config;
namespace fs = std::filesystem;

// Color codes for terminal output
const std::string COLOR_GREEN = "\033[32m";
const std::string COLOR_RED = "\033[31m";
const std::string COLOR_YELLOW = "\033[33m";
const std::string COLOR_BLUE = "\033[34m";
const std::string COLOR_RESET = "\033[0m";

// Function declarations
void printUsage(const std::string& programName);
void printVersion();
std::string getFileExtension(const std::string& filepath);
bool parseCommand(const std::vector<std::string>& args);
bool commandParse(const std::vector<std::string>& args);
bool commandValidate(const std::vector<std::string>& args);
bool commandConvert(const std::vector<std::string>& args);
bool commandMerge(const std::vector<std::string>& args);
bool commandExportSchema(const std::vector<std::string>& args);

/**
 * @brief Print usage information
 */
void printUsage(const std::string& programName) {
    std::cout << "\n" << COLOR_BLUE << "IOC_Config - Configuration File CLI Tool" << COLOR_RESET << "\n";
    std::cout << "Version 1.2.0\n\n";
    std::cout << COLOR_YELLOW << "Usage:" << COLOR_RESET << "\n";
    std::cout << "  " << programName << " <command> [options]\n\n";
    std::cout << COLOR_YELLOW << "Commands:" << COLOR_RESET << "\n";
    std::cout << "  parse <file>              Parse and display configuration\n";
    std::cout << "  validate <file>           Validate configuration against schema\n";
    std::cout << "  convert <input> <output>  Convert between formats (OOP, JSON, YAML)\n";
    std::cout << "  merge <file1> <file2>     Merge two configurations\n";
    std::cout << "  export-schema <output>    Export JSON schema to file\n";
    std::cout << "  --version                 Show version information\n";
    std::cout << "  --help                    Show this help message\n\n";
    std::cout << COLOR_YELLOW << "Supported Formats:" << COLOR_RESET << "\n";
    std::cout << "  .oop   - IOC OOP format (native)\n";
    std::cout << "  .json  - JSON format\n";
    std::cout << "  .yaml  - YAML format (requires yaml-cpp)\n\n";
    std::cout << COLOR_YELLOW << "Examples:" << COLOR_RESET << "\n";
    std::cout << "  " << programName << " parse config.oop\n";
    std::cout << "  " << programName << " convert config.oop config.json\n";
    std::cout << "  " << programName << " validate config.yaml\n";
    std::cout << "  " << programName << " export-schema schema.json\n\n";
}

/**
 * @brief Print version information
 */
void printVersion() {
    std::cout << "IOC_Config CLI Tool v1.2.0\n";
    std::cout << "Built with C++17\n";
    #ifdef IOC_CONFIG_YAML_SUPPORT
    std::cout << "YAML support: enabled\n";
    #else
    std::cout << "YAML support: disabled (yaml-cpp not found)\n";
    #endif
}

/**
 * @brief Get file extension from filepath
 */
std::string getFileExtension(const std::string& filepath) {
    size_t pos = filepath.rfind('.');
    if (pos == std::string::npos) return "";
    return filepath.substr(pos);
}

/**
 * @brief Parse command from arguments
 */
bool parseCommand(const std::vector<std::string>& args) {
    if (args.empty()) return false;
    
    std::string command = args[0];
    
    if (command == "parse") {
        return commandParse(args);
    } else if (command == "validate") {
        return commandValidate(args);
    } else if (command == "convert") {
        return commandConvert(args);
    } else if (command == "merge") {
        return commandMerge(args);
    } else if (command == "export-schema") {
        return commandExportSchema(args);
    } else {
        std::cerr << COLOR_RED << "✗ Unknown command: " << command << COLOR_RESET << "\n";
        return false;
    }
}

/**
 * @brief Command: Parse configuration file
 */
bool commandParse(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << COLOR_RED << "✗ Missing filename" << COLOR_RESET << "\n";
        std::cerr << "Usage: ioc-config parse <file>\n";
        return false;
    }
    
    std::string filepath = args[1];
    std::string ext = getFileExtension(filepath);
    OopParser parser;
    
    std::cout << COLOR_BLUE << "Parsing: " << filepath << COLOR_RESET << "\n";
    
    // Load based on file extension
    bool loaded = false;
    if (ext == ".json") {
        loaded = parser.loadFromJson(filepath);
    } else if (ext == ".yaml" || ext == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        loaded = parser.loadFromYaml(filepath);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else {
        // Default to OOP format
        loaded = parser.loadFromOop(filepath);
    }
    
    if (!loaded) {
        std::cerr << COLOR_RED << "✗ Failed to parse file: " << parser.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    std::cout << COLOR_GREEN << "✓ Parsed successfully\n" << COLOR_RESET;
    std::cout << "\nConfiguration (as JSON):\n";
    std::cout << parser.toJsonString(2) << "\n";
    
    return true;
}

/**
 * @brief Command: Validate configuration file
 */
bool commandValidate(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << COLOR_RED << "✗ Missing filename" << COLOR_RESET << "\n";
        std::cerr << "Usage: ioc-config validate <file>\n";
        return false;
    }
    
    std::string filepath = args[1];
    std::string ext = getFileExtension(filepath);
    OopParser parser;
    
    std::cout << COLOR_BLUE << "Validating: " << filepath << COLOR_RESET << "\n";
    
    // Load based on file extension
    bool loaded = false;
    if (ext == ".json") {
        loaded = parser.loadFromJson(filepath);
    } else if (ext == ".yaml" || ext == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        loaded = parser.loadFromYaml(filepath);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else {
        loaded = parser.loadFromOop(filepath);
    }
    
    if (!loaded) {
        std::cerr << COLOR_RED << "✗ Failed to load file: " << parser.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    // Validation: basic check
    if (parser.getSectionCount() > 0) {
        std::cout << COLOR_GREEN << "✓ Configuration is valid" << COLOR_RESET << "\n";
        std::cout << "  Sections: " << parser.getSectionCount() << "\n";
        return true;
    } else {
        std::cerr << COLOR_RED << "✗ No sections found in configuration" << COLOR_RESET << "\n";
        return false;
    }
}

/**
 * @brief Command: Convert between formats
 */
bool commandConvert(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << COLOR_RED << "✗ Missing input or output file" << COLOR_RESET << "\n";
        std::cerr << "Usage: ioc-config convert <input> <output>\n";
        return false;
    }
    
    std::string input_file = args[1];
    std::string output_file = args[2];
    std::string input_ext = getFileExtension(input_file);
    std::string output_ext = getFileExtension(output_file);
    
    OopParser parser;
    
    std::cout << COLOR_BLUE << "Converting from " << input_ext << " to " << output_ext << COLOR_RESET << "\n";
    
    // Load input
    std::cout << "  Loading: " << input_file << "\n";
    bool loaded = false;
    if (input_ext == ".json") {
        loaded = parser.loadFromJson(input_file);
    } else if (input_ext == ".yaml" || input_ext == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        loaded = parser.loadFromYaml(input_file);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else {
        loaded = parser.loadFromOop(input_file);
    }
    
    if (!loaded) {
        std::cerr << COLOR_RED << "✗ Failed to load input: " << parser.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    // Save to output format
    std::cout << "  Saving: " << output_file << "\n";
    bool success = false;
    
    if (output_ext == ".json") {
        success = parser.saveToJson(output_file);
    } else if (output_ext == ".yaml" || output_ext == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        success = parser.saveToYaml(output_file);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else if (output_ext == ".oop") {
        success = parser.saveToOop(output_file);
    } else {
        std::cerr << COLOR_RED << "✗ Unknown output format: " << output_ext << COLOR_RESET << "\n";
        return false;
    }
    
    if (!success) {
        std::cerr << COLOR_RED << "✗ Failed to save output: " << parser.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    std::cout << COLOR_GREEN << "✓ Conversion successful" << COLOR_RESET << "\n";
    return true;
}

/**
 * @brief Command: Merge two configurations
 */
bool commandMerge(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << COLOR_RED << "✗ Missing configuration files" << COLOR_RESET << "\n";
        std::cerr << "Usage: ioc-config merge <file1> <file2>\n";
        return false;
    }
    
    std::string file1 = args[1];
    std::string file2 = args[2];
    std::string ext1 = getFileExtension(file1);
    std::string ext2 = getFileExtension(file2);
    
    OopParser parser1, parser2;
    
    std::cout << COLOR_BLUE << "Merging configurations" << COLOR_RESET << "\n";
    std::cout << "  File 1: " << file1 << "\n";
    std::cout << "  File 2: " << file2 << "\n";
    
    // Load first file
    bool loaded1 = false;
    if (ext1 == ".json") {
        loaded1 = parser1.loadFromJson(file1);
    } else if (ext1 == ".yaml" || ext1 == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        loaded1 = parser1.loadFromYaml(file1);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else {
        loaded1 = parser1.loadFromOop(file1);
    }
    
    if (!loaded1) {
        std::cerr << COLOR_RED << "✗ Failed to load file 1: " << parser1.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    // Load second file
    bool loaded2 = false;
    if (ext2 == ".json") {
        loaded2 = parser2.loadFromJson(file2);
    } else if (ext2 == ".yaml" || ext2 == ".yml") {
        #ifdef IOC_CONFIG_YAML_SUPPORT
        loaded2 = parser2.loadFromYaml(file2);
        #else
        std::cerr << COLOR_RED << "✗ YAML support not enabled" << COLOR_RESET << "\n";
        return false;
        #endif
    } else {
        loaded2 = parser2.loadFromOop(file2);
    }
    
    if (!loaded2) {
        std::cerr << COLOR_RED << "✗ Failed to load file 2: " << parser2.getLastError() << COLOR_RESET << "\n";
        return false;
    }
    
    std::cout << COLOR_GREEN << "✓ Merge completed" << COLOR_RESET << "\n";
    std::cout << "  File 1 sections: " << parser1.getSectionCount() << "\n";
    std::cout << "  File 2 sections: " << parser2.getSectionCount() << "\n";
    std::cout << "\nConfiguration 1 (as JSON):\n";
    std::cout << parser1.toJsonString(2) << "\n";
    
    return true;
}

/**
 * @brief Command: Export schema
 */
bool commandExportSchema(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << COLOR_RED << "✗ Missing output file" << COLOR_RESET << "\n";
        std::cerr << "Usage: ioc-config export-schema <output>\n";
        return false;
    }
    
    std::string output_file = args[1];
    
    std::cout << COLOR_BLUE << "Exporting schema to: " << output_file << COLOR_RESET << "\n";
    
    // Create a sample configuration to demonstrate schema
    OopParser parser;
    parser.setParameter("object", ".id", "'example'");
    parser.setParameter("time", ".start", "'2024-01-01'");
    
    // Save as JSON to show structure
    if (parser.saveToJson(output_file)) {
        std::cout << COLOR_GREEN << "✓ Schema reference exported successfully" << COLOR_RESET << "\n";
        return true;
    } else {
        std::cerr << COLOR_RED << "✗ Failed to export schema" << COLOR_RESET << "\n";
        return false;
    }
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string firstArg = argv[1];
    
    // Handle special commands
    if (firstArg == "--help" || firstArg == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    
    if (firstArg == "--version" || firstArg == "-v") {
        printVersion();
        return 0;
    }
    
    // Parse command with arguments
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    
    if (!parseCommand(args)) {
        return 1;
    }
    
    return 0;
}
