/**
 * @file oop_parser.cpp
 * @brief IOC_Config - OOP File Parser Library Implementation
 * 
 * Implementation of OOP file parsing, JSON conversion, schema validation,
 * and configuration building functionality.
 * 
 * @author Michele Bigi (mikbigi@gmail.com)
 * @version 1.2.0
 * @date 2025-12-02
 * @license MIT
 */

#include "ioc_config/oop_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <set>

// Include nlohmann/json for JSON support
#include <nlohmann/json.hpp>

// Include yaml-cpp for YAML support (if available)
#ifdef IOC_CONFIG_YAML_SUPPORT
#include <yaml-cpp/yaml.h>
#endif

// Include toml11 for TOML support (if available)
#ifdef IOC_CONFIG_TOML_SUPPORT
#include <toml.hpp>
#endif

using json = nlohmann::json;

namespace ioc_config {

// ============ ConfigParameter Implementation ============

std::string ConfigParameter::asString() const {
    return value;
}

double ConfigParameter::asDouble() const {
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        throw std::runtime_error("Cannot convert '" + value + "' to double: " + e.what());
    }
}

int ConfigParameter::asInt() const {
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        throw std::runtime_error("Cannot convert '" + value + "' to int: " + e.what());
    }
}

bool ConfigParameter::asBoolean() const {
    std::string lower_val = value;
    std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);
    
    if (lower_val == ".true." || lower_val == "true" || lower_val == "1" || lower_val == "yes") {
        return true;
    } else if (lower_val == ".false." || lower_val == "false" || lower_val == "0" || lower_val == "no") {
        return false;
    }
    throw std::runtime_error("Cannot convert '" + value + "' to boolean");
}

std::vector<std::string> ConfigParameter::asStringVector() const {
    return OopParser::split(value, ',');
}

// ============ RangeConstraint Implementation ============

bool RangeConstraint::parseExpression(const std::string& expr) {
    constraint_expr = expr;
    std::string trimmed = expr;
    // Remove spaces
    trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), ' '), trimmed.end());

    // Case 1: "1..100" or "1..N"
    size_t dot_pos = trimmed.find("..");
    if (dot_pos != std::string::npos) {
        try {
            std::string min_str = trimmed.substr(0, dot_pos);
            std::string max_str = trimmed.substr(dot_pos + 2);
            
            min_value = std::stod(min_str);
            min_inclusive = true;
            
            if (max_str == "N") {
                is_range_to_catalog = true;
                max_value = 1e308;  // Large number
            } else {
                max_value = std::stod(max_str);
            }
            max_inclusive = true;
            enabled = true;
            return true;
        } catch (...) {
            return false;
        }
    }

    // Case 3: "5 < d < 30" or "30 > d > 5" (DOUBLE INEQUALITIES)
    // Check this BEFORE Case 2 to avoid ambiguity
    size_t d_pos = trimmed.find('d');
    if (d_pos != std::string::npos && d_pos > 0 && d_pos < trimmed.length() - 1) {
        // Look for comparison operators on both sides of 'd'
        std::string before_d = trimmed.substr(0, d_pos);
        std::string after_d = trimmed.substr(d_pos + 1);
        
        // Check if both sides end/start with a comparison operator
        char op_before = before_d.empty() ? '\0' : before_d.back();
        char op_after = after_d.empty() ? '\0' : after_d.front();
        
        if ((op_before == '<' || op_before == '>') && (op_after == '<' || op_after == '>')) {
            // This is a double inequality
            try {
                // Extract the numbers
                size_t len_before = (op_before == '<' || op_before == '>') ? 1 : 0;
                size_t len_after = (op_after == '<' || op_after == '>') ? 1 : 0;
                
                std::string min_str = before_d.substr(0, before_d.length() - len_before);
                std::string max_str = after_d.substr(len_after);
                
                double min_val = std::stod(min_str);
                double max_val = std::stod(max_str);
                
                // Determine inclusivity based on operators
                // "5 < d" means min is exclusive; "5 <= d" means min is inclusive
                // "d < 30" means max is exclusive; "d <= 30" means max is inclusive
                bool min_is_less_than = (op_before == '<');
                bool max_is_less_than = (op_after == '<');
                
                min_value = min_val;
                max_value = max_val;
                min_inclusive = !min_is_less_than;  // If "<", exclusive. If ">", it's the wrong operator (swap values)
                max_inclusive = !max_is_less_than;  // If "<", exclusive. If ">", should not happen in "5 < d < 30"
                
                // Special case: if pattern is "30 > d > 5", swap and adjust
                if (op_before == '>') {
                    std::swap(min_value, max_value);
                    min_inclusive = true;  // "30 > d" means d can equal 30
                    max_inclusive = true;  // "d > 5" means d can equal 5
                }
                
                enabled = true;
                return true;
            } catch (...) {
                // Fall through to Case 2
            }
        }
    }

    // Case 2: "d >= 4", "d <= 100", "d > 5", "d < 1000"
    if (trimmed.find("d>=") != std::string::npos) {
        try {
            std::string val_str = trimmed.substr(trimmed.find("d>=") + 3);
            min_value = std::stod(val_str);
            min_inclusive = true;
            max_value = 1e308;
            enabled = true;
            return true;
        } catch (...) {
            return false;
        }
    }
    if (trimmed.find("d<=") != std::string::npos) {
        try {
            std::string val_str = trimmed.substr(trimmed.find("d<=") + 3);
            max_value = std::stod(val_str);
            max_inclusive = true;
            min_value = -1e308;
            enabled = true;
            return true;
        } catch (...) {
            return false;
        }
    }
    if (trimmed.find("d>") != std::string::npos && trimmed.find("d>=") == std::string::npos) {
        try {
            std::string val_str = trimmed.substr(trimmed.find("d>") + 2);
            min_value = std::stod(val_str);
            min_inclusive = false;
            max_value = 1e308;
            enabled = true;
            return true;
        } catch (...) {
            return false;
        }
    }
    if (trimmed.find("d<") != std::string::npos && trimmed.find("d<=") == std::string::npos) {
        try {
            std::string val_str = trimmed.substr(trimmed.find("d<") + 2);
            max_value = std::stod(val_str);
            max_inclusive = false;
            min_value = -1e308;
            enabled = true;
            return true;
        } catch (...) {
            return false;
        }
    }

    return false;
}

bool RangeConstraint::isSatisfied(double value) const {
    if (!enabled) return true;

    bool min_ok = min_inclusive ? (value >= min_value) : (value > min_value);
    bool max_ok = max_inclusive ? (value <= max_value) : (value < max_value);

    return min_ok && max_ok;
}

std::string RangeConstraint::toString() const {
    if (!enabled) return "no constraint";
    return constraint_expr;
}

// ============ ParameterSpec Implementation ============

bool ParameterSpec::isValid(const std::string& value) const {
    // Check allowed values first
    if (!allowed_values.empty()) {
        return std::find(allowed_values.begin(), allowed_values.end(), value) != allowed_values.end();
    }

    // Check constraint if numeric
    try {
        double val = std::stod(value);
        return constraint.isSatisfied(val);
    } catch (...) {
        // Non-numeric value, pass if no constraint
        return !constraint.enabled;
    }
}

// ============ OopParser Implementation (extended) ============

std::string ConfigSectionData::sectionTypeToString(SectionType type) {
    switch (type) {
        case SectionType::OBJECT: return "object";
        case SectionType::PROPAGATION: return "propagation";
        case SectionType::ASTEROIDS: return "asteroids";
        case SectionType::TIME: return "time";
        case SectionType::SEARCH: return "search";
        case SectionType::DATABASE: return "database";
        case SectionType::GAIA: return "gaia";
        case SectionType::OBSERVER: return "observer";
        case SectionType::OUTPUT: return "output";
        case SectionType::PERFORMANCE: return "performance";
        case SectionType::OCCULTATION: return "occultation";
        case SectionType::FILTERS: return "filters";
        default: return "unknown";
    }
}

SectionType ConfigSectionData::stringToSectionType(const std::string& name) {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    
    if (lower_name == "object") return SectionType::OBJECT;
    if (lower_name == "propag" || lower_name == "propagation") return SectionType::PROPAGATION;
    if (lower_name == "asteroids") return SectionType::ASTEROIDS;
    if (lower_name == "time") return SectionType::TIME;
    if (lower_name == "search") return SectionType::SEARCH;
    if (lower_name == "database") return SectionType::DATABASE;
    if (lower_name == "gaia") return SectionType::GAIA;
    if (lower_name == "observer") return SectionType::OBSERVER;
    if (lower_name == "output") return SectionType::OUTPUT;
    if (lower_name == "performance") return SectionType::PERFORMANCE;
    if (lower_name == "occultation") return SectionType::OCCULTATION;
    if (lower_name == "filters") return SectionType::FILTERS;
    
    return SectionType::UNKNOWN;
}

ConfigParameter* ConfigSectionData::getParameter(const std::string& key) {
    auto it = parameters.find(key);
    return it != parameters.end() ? &it->second : nullptr;
}

const ConfigParameter* ConfigSectionData::getParameter(const std::string& key) const {
    auto it = parameters.find(key);
    return it != parameters.end() ? &it->second : nullptr;
}

// ============ OopParser Implementation ============

OopParser::OopParser() : lastError_("") {}

OopParser::OopParser(const std::string& filepath) : lastError_("") {
    loadFromOop(filepath);
}

bool OopParser::loadFromOop(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        lastError_ = "Cannot open file: " + filepath;
        return false;
    }

    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    clear();
    std::string line;
    ConfigSectionData currentSection;
    currentSection.type = SectionType::UNKNOWN;

    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || isComment(line)) {
            continue;
        }

        // Check if this is a new section
        std::string sectionName = isSection(line);
        if (!sectionName.empty()) {
            // Save previous section if it has content
            if (!currentSection.parameters.empty()) {
                sections_.push_back(currentSection);
            }
            // Start new section
            currentSection.name = sectionName;
            currentSection.type = ConfigSectionData::stringToSectionType(sectionName);
            currentSection.parameters.clear();
        } else {
            // Parse parameter
            if (!parseLine(line, currentSection)) {
                lastError_ = "Error parsing line: " + line;
                return false;
            }
        }
    }

    // Save last section
    if (!currentSection.parameters.empty()) {
        sections_.push_back(currentSection);
    }

    file.close();
    return true;
}

bool OopParser::saveToOop(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        lastError_ = "Cannot open file for writing: " + filepath;
        return false;
    }

    for (const auto& section : sections_) {
        // Write section header
        file << section.name << ".\n";

        // Write parameters
        for (const auto& [key, param] : section.parameters) {
            file << "\t" << key << " = " << param.value << "\n";
        }

        file << "\n";
    }

    file.close();
    return true;
}

bool OopParser::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        lastError_ = "Cannot open JSON file: " + filepath;
        return false;
    }

    try {
        json j;
        file >> j;
        file.close();

        clear();

        for (auto& [section_name, section_obj] : j.items()) {
            ConfigSectionData section;
            section.name = section_name;
            section.type = ConfigSectionData::stringToSectionType(section_name);

            if (section_obj.is_object()) {
                for (auto& [key, value] : section_obj.items()) {
                    ConfigParameter param;
                    param.key = key;
                    param.value = value.dump();
                    param.type = detectType(value.dump());
                    section.parameters[key] = param;
                }
            }

            sections_.push_back(section);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("JSON parsing error: ") + e.what();
        return false;
    }
}

bool OopParser::saveToJson(const std::string& filepath) const {
    try {
        json j;

        for (const auto& section : sections_) {
            json section_obj;
            for (const auto& [key, param] : section.parameters) {
                section_obj[key] = json::parse(param.value);
            }
            j[section.name] = section_obj;
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            lastError_ = "Cannot open file for writing: " + filepath;
            return false;
        }

        file << j.dump(2) << std::endl;
        file.close();
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("JSON serialization error: ") + e.what();
        return false;
    }
}

std::vector<ConfigSectionData> OopParser::getAllSections() const {
    return sections_;
}

ConfigSectionData* OopParser::getSection(SectionType type) {
    for (auto& section : sections_) {
        if (section.type == type) {
            return &section;
        }
    }
    return nullptr;
}

const ConfigSectionData* OopParser::getSection(SectionType type) const {
    for (const auto& section : sections_) {
        if (section.type == type) {
            return &section;
        }
    }
    return nullptr;
}

ConfigSectionData* OopParser::getSection(const std::string& name) {
    for (auto& section : sections_) {
        if (section.name == name) {
            return &section;
        }
    }
    return nullptr;
}

const ConfigSectionData* OopParser::getSection(const std::string& name) const {
    for (const auto& section : sections_) {
        if (section.name == name) {
            return &section;
        }
    }
    return nullptr;
}

bool OopParser::setParameter(const std::string& sectionName, 
                             const std::string& paramKey, 
                             const std::string& value) {
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    ConfigSectionData* section = getSection(sectionName);
    if (!section) {
        // Create new section if it doesn't exist
        ConfigSectionData newSection;
        newSection.name = sectionName;
        newSection.type = ConfigSectionData::stringToSectionType(sectionName);
        sections_.push_back(newSection);
        section = &sections_.back();
    }

    ConfigParameter param;
    param.key = paramKey;
    param.value = value;
    param.type = detectType(value);
    
    section->parameters[paramKey] = param;
    return true;
}

ConfigParameter* OopParser::findParameter(const std::string& paramKey) {
    for (auto& section : sections_) {
        ConfigParameter* param = section.getParameter(paramKey);
        if (param) {
            return param;
        }
    }
    return nullptr;
}

const ConfigParameter* OopParser::findParameter(const std::string& paramKey) const {
    for (const auto& section : sections_) {
        const ConfigParameter* param = section.getParameter(paramKey);
        if (param) {
            return param;
        }
    }
    return nullptr;
}

bool OopParser::validate(std::vector<std::string>& errors) const {
    errors.clear();

    // Check for required sections
    const std::vector<std::string> required_sections = {"object", "time", "search"};
    
    for (const auto& req_section : required_sections) {
        bool found = false;
        for (const auto& section : sections_) {
            if (section.name == req_section) {
                found = true;
                break;
            }
        }
        if (!found) {
            errors.push_back("Missing required section: " + req_section);
        }
    }

    return errors.empty();
}

void OopParser::clear() {
    sections_.clear();
    lastError_ = "";
}

size_t OopParser::getSectionCount() const {
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    return sections_.size();
}

std::string OopParser::getLastError() const {
    return lastError_;
}

bool OopParser::parseLine(const std::string& line, ConfigSectionData& section) {
    // Find the assignment operator
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
        return false;
    }

    // Extract key and value
    std::string key = trim(line.substr(0, eq_pos));
    std::string value = trim(line.substr(eq_pos + 1));

    // Remove leading dot from key if present
    if (!key.empty() && key[0] == '.') {
        key = key.substr(1);
    }

    // Remove quotes from value if present
    if (!value.empty()) {
        if ((value.front() == '\'' && value.back() == '\'') ||
            (value.front() == '"' && value.back() == '"')) {
            value = value.substr(1, value.length() - 2);
        }
    }

    // Create parameter
    ConfigParameter param;
    param.key = key;
    param.value = value;
    param.type = detectType(value);

    section.parameters[key] = param;
    return true;
}

std::string OopParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> OopParser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

bool OopParser::isComment(const std::string& line) {
    return !line.empty() && line[0] == '!';
}

std::string OopParser::isSection(const std::string& line) {
    if (line.empty()) return "";
    
    // Check if line ends with a dot
    if (line.back() == '.') {
        // Return section name without the dot
        return line.substr(0, line.length() - 1);
    }
    
    return "";
}

std::vector<std::string> OopParser::parseArrayValue(const std::string& value) {
    // Remove brackets if present
    std::string val = value;
    if (val.front() == '[' && val.back() == ']') {
        val = val.substr(1, val.length() - 2);
    }

    // Split by comma
    return split(val, ',');
}

std::string OopParser::detectType(const std::string& value) {
    std::string trimmed = trim(value);

    // Check for array
    if (trimmed.front() == '[' && trimmed.back() == ']') {
        return "array";
    }

    // Check for boolean
    std::string lower = trimmed;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == ".true." || lower == ".false." || lower == "true" || lower == "false") {
        return "bool";
    }

    // Check for number
    try {
        size_t idx;
        std::stod(trimmed, &idx);
        if (idx == trimmed.length()) {
            // Check if it's int or float
            if (trimmed.find('.') != std::string::npos) {
                return "float";
            } else {
                return "int";
            }
        }
    } catch (...) {
        // Not a number
    }

    // Default to string
    return "string";
}

// ============ JSON Native Support Methods ============

bool OopParser::loadFromJsonString(const std::string& jsonString) {
    try {
        json j = json::parse(jsonString);
        return loadFromJsonObject(j);
    } catch (const std::exception& e) {
        lastError_ = std::string("JSON parsing error: ") + e.what();
        return false;
    }
}

std::string OopParser::saveToJsonString() const {
    return toJsonString();
}

bool OopParser::loadFromJsonObject(const nlohmann::json& jsonObj) {
    try {
        clear();

        for (auto& [section_name, section_obj] : jsonObj.items()) {
            ConfigSectionData section;
            section.name = section_name;
            section.type = ConfigSectionData::stringToSectionType(section_name);

            if (section_obj.is_object()) {
                for (auto& [key, value] : section_obj.items()) {
                    ConfigParameter param;
                    param.key = key;
                    
                    // Convert value to string representation
                    if (value.is_string()) {
                        param.value = "'" + value.get<std::string>() + "'";
                        param.type = "string";
                    } else if (value.is_number_float()) {
                        param.value = std::to_string(value.get<double>());
                        param.type = "float";
                    } else if (value.is_number_integer()) {
                        param.value = std::to_string(value.get<long>());
                        param.type = "int";
                    } else if (value.is_boolean()) {
                        param.value = value.get<bool>() ? ".TRUE." : ".FALSE.";
                        param.type = "bool";
                    } else if (value.is_array()) {
                        param.value = value.dump();
                        param.type = "array";
                    } else {
                        param.value = value.dump();
                        param.type = "unknown";
                    }
                    
                    section.parameters[key] = param;
                }
            }

            sections_.push_back(section);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("JSON object loading error: ") + e.what();
        return false;
    }
}

nlohmann::json OopParser::saveToJsonObject() const {
    json result;

    for (const auto& section : sections_) {
        json section_obj;
        
        for (const auto& [key, param] : section.parameters) {
            // Parse value back to appropriate JSON type
            if (param.type == "string") {
                // Remove quotes
                std::string val = param.value;
                if (!val.empty() && val.front() == '\'' && val.back() == '\'') {
                    val = val.substr(1, val.length() - 2);
                }
                section_obj[key] = val;
            } else if (param.type == "float") {
                try {
                    section_obj[key] = std::stod(param.value);
                } catch (...) {
                    section_obj[key] = param.value;
                }
            } else if (param.type == "int") {
                try {
                    section_obj[key] = std::stol(param.value);
                } catch (...) {
                    section_obj[key] = param.value;
                }
            } else if (param.type == "bool") {
                std::string lower = param.value;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                section_obj[key] = (lower == ".true." || lower == "true" || lower == "1");
            } else if (param.type == "array") {
                try {
                    section_obj[key] = json::parse(param.value);
                } catch (...) {
                    section_obj[key] = param.value;
                }
            } else {
                section_obj[key] = param.value;
            }
        }
        
        result[section.name] = section_obj;
    }

    return result;
}

nlohmann::json OopParser::toJson() const {
    return saveToJsonObject();
}

bool OopParser::fromJson(const nlohmann::json& jsonObj) {
    return loadFromJsonObject(jsonObj);
}

bool OopParser::validateAgainstSchema(const nlohmann::json& schemaJson, 
                                      std::vector<std::string>& errors) const {
    errors.clear();

    // Simple schema validation
    if (!schemaJson.contains("required")) {
        return true;
    }

    auto required = schemaJson["required"];
    if (required.is_array()) {
        for (const auto& req_field : required) {
            if (req_field.is_string()) {
                const std::string field = req_field.get<std::string>();
                bool found = false;
                
                for (const auto& section : sections_) {
                    if (section.getParameter(field) != nullptr) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    errors.push_back("Missing required field: " + field);
                }
            }
        }
    }

    return errors.empty();
}

std::string OopParser::toJsonString(int indent) const {
    json j = saveToJsonObject();
    return j.dump(indent);
}

// ============ Schema and Advanced Validation ============

bool OopParser::validateWithSchema(const ConfigSchema& schema, 
                                   std::vector<std::string>& errors) const {
    errors.clear();

    // Check required sections
    for (const auto& [section_name, section_spec] : schema.sections) {
        if (section_spec.required) {
            auto section = getSection(section_name);
            if (!section) {
                errors.push_back("Missing required section: " + section_name);
                continue;
            }

            // Check required parameters
            for (const auto& [param_key, param_spec] : section_spec.params) {
                if (param_spec.required) {
                    auto param = section->getParameter(param_key);
                    if (!param) {
                        errors.push_back("Missing required parameter '" + param_key + 
                                       "' in section '" + section_name + "'");
                    } else if (!param_spec.isValid(param->value)) {
                        errors.push_back("Parameter '" + param_key + 
                                       "' in section '" + section_name + 
                                       "' failed validation: " + param_spec.constraint.toString());
                    }
                }
            }
        }
    }

    return errors.empty();
}

void OopParser::setSchema(const ConfigSchema& schema) {
    schema_ = std::make_unique<ConfigSchema>(schema);
}

const ConfigSchema* OopParser::getSchema() const {
    return schema_.get();
}

ConfigSchema OopParser::createDefaultSchema() {
    ConfigSchema schema;
    schema.name = "IOoccultCalc Default";
    schema.version = "1.0";

    // Object section (required)
    SectionSpec object_spec;
    object_spec.name = "object";
    object_spec.required = true;
    object_spec.description = "Target object specification";
    
    ParameterSpec id_spec;
    id_spec.key = "id";
    id_spec.required = true;
    id_spec.description = "Object identifier";
    object_spec.addParameter(id_spec);
    
    ParameterSpec name_spec;
    name_spec.key = "name";
    name_spec.required = true;
    name_spec.description = "Object name";
    object_spec.addParameter(name_spec);
    
    schema.addSection(object_spec);

    // Time section (required)
    SectionSpec time_spec;
    time_spec.name = "time";
    time_spec.required = true;
    time_spec.description = "Time range specification";
    
    ParameterSpec start_date_spec;
    start_date_spec.key = "start_date";
    start_date_spec.required = true;
    start_date_spec.description = "Start date (YYYY-MM-DD)";
    time_spec.addParameter(start_date_spec);
    
    ParameterSpec end_date_spec;
    end_date_spec.key = "end_date";
    end_date_spec.required = true;
    end_date_spec.description = "End date (YYYY-MM-DD)";
    time_spec.addParameter(end_date_spec);
    
    schema.addSection(time_spec);

    // Search section (required)
    SectionSpec search_spec;
    search_spec.name = "search";
    search_spec.required = true;
    search_spec.description = "Occultation search parameters";
    
    ParameterSpec mag_spec;
    mag_spec.key = "max_magnitude";
    mag_spec.required = false;
    mag_spec.description = "Maximum magnitude";
    mag_spec.constraint.parseExpression("d < 20");
    search_spec.addParameter(mag_spec);
    
    schema.addSection(search_spec);

    // Database section (optional)
    SectionSpec db_spec;
    db_spec.name = "database";
    db_spec.required = false;
    db_spec.description = "Database configuration";
    schema.addSection(db_spec);

    // Propagation section (optional)
    SectionSpec propag_spec;
    propag_spec.name = "propag";
    propag_spec.required = false;
    propag_spec.description = "Propagation settings";
    
    ParameterSpec step_spec;
    step_spec.key = "step_size";
    step_spec.required = false;
    step_spec.description = "Integration step size";
    step_spec.constraint.parseExpression("0.001..10");
    propag_spec.addParameter(step_spec);
    
    schema.addSection(propag_spec);

    return schema;
}

// ============ ConfigSchema JSON Export ============

nlohmann::json ConfigSchema::toJsonSchema() const {
    json schema;
    
    schema["$schema"] = "http://json-schema.org/draft-07/schema#";
    schema["title"] = name;
    schema["version"] = version;
    schema["type"] = "object";
    
    json required_array = json::array();
    json properties = json::object();
    
    // Add each section
    for (const auto& [section_name, section_spec] : sections) {
        json section_prop;
        section_prop["type"] = "object";
        section_prop["description"] = section_spec.description;
        
        json section_required = json::array();
        json section_properties = json::object();
        
        // Add parameters for this section
        for (const auto& [param_key, param_spec] : section_spec.params) {
            json param_prop;
            param_prop["description"] = param_spec.description;
            
            // Set type based on constraint or default to string
            if (param_spec.constraint.enabled) {
                param_prop["type"] = "number";
                
                // Add constraint information
                if (param_spec.constraint.min_value > -1e308) {
                    if (param_spec.constraint.min_inclusive) {
                        param_prop["minimum"] = param_spec.constraint.min_value;
                    } else {
                        param_prop["exclusiveMinimum"] = param_spec.constraint.min_value;
                    }
                }
                
                if (param_spec.constraint.max_value < 1e308) {
                    if (param_spec.constraint.max_inclusive) {
                        param_prop["maximum"] = param_spec.constraint.max_value;
                    } else {
                        param_prop["exclusiveMaximum"] = param_spec.constraint.max_value;
                    }
                }
                
                param_prop["constraint"] = param_spec.constraint.constraint_expr;
            } else if (!param_spec.allowed_values.empty()) {
                // Enum type
                json enum_values = json::array();
                for (const auto& val : param_spec.allowed_values) {
                    enum_values.push_back(val);
                }
                param_prop["enum"] = enum_values;
            } else {
                param_prop["type"] = "string";
            }
            
            section_properties[param_key] = param_prop;
            
            if (param_spec.required) {
                section_required.push_back(param_key);
            }
        }
        
        section_prop["properties"] = section_properties;
        if (!section_required.empty()) {
            section_prop["required"] = section_required;
        }
        
        properties[section_name] = section_prop;
        
        if (section_spec.required) {
            required_array.push_back(section_name);
        }
    }
    
    schema["properties"] = properties;
    if (!required_array.empty()) {
        schema["required"] = required_array;
    }
    
    return schema;
}

bool ConfigSchema::saveJsonSchema(const std::string& filepath) const {
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        json schema = toJsonSchema();
        file << schema.dump(2);
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

std::string ConfigSchema::toJsonSchemaString(int indent) const {
    json schema = toJsonSchema();
    return schema.dump(indent);
}

// ============ ConfigBuilder Implementation ============

ConfigBuilder::ConfigBuilder() : currentSectionIndex_(-1) {}

ConfigBuilder& ConfigBuilder::addSection(const std::string& sectionName) {
    ConfigSectionData section;
    section.name = sectionName;
    section.type = ConfigSectionData::stringToSectionType(sectionName);
    section.parameters.clear();
    
    sections_.push_back(section);
    currentSection_ = sectionName;
    currentSectionIndex_ = sections_.size() - 1;
    
    return *this;
}

ConfigBuilder& ConfigBuilder::addParameter(const std::string& key, const std::string& value) {
    if (currentSectionIndex_ < 0 || currentSectionIndex_ >= static_cast<int>(sections_.size())) {
        throw std::runtime_error("No section currently selected. Call addSection() first.");
    }
    
    ConfigParameter param;
    param.key = key;
    param.value = value;
    param.type = OopParser::detectType(value);
    
    sections_[currentSectionIndex_].parameters[key] = param;
    
    return *this;
}

ConfigBuilder& ConfigBuilder::endSection() {
    currentSectionIndex_ = -1;
    currentSection_ = "";
    return *this;
}

ConfigBuilder& ConfigBuilder::addParameters(const std::map<std::string, std::string>& params) {
    for (const auto& [key, value] : params) {
        addParameter(key, value);
    }
    return *this;
}

std::unique_ptr<OopParser> ConfigBuilder::build() const {
    auto parser = std::make_unique<OopParser>();
    for (const auto& section : sections_) {
        // We need to set parameters through the parser
        for (const auto& [key, param] : section.parameters) {
            parser->setParameter(section.name, key, param.value);
        }
    }
    return parser;
}

std::unique_ptr<OopParser> ConfigBuilder::getParser() const {
    return build();
}

ConfigBuilder& ConfigBuilder::clear() {
    sections_.clear();
    currentSectionIndex_ = -1;
    currentSection_ = "";
    return *this;
}

ConfigBuilder& ConfigBuilder::addSectionFrom(const OopParser& parser, const std::string& sectionName) {
    auto section = parser.getSection(sectionName);
    if (!section) {
        throw std::runtime_error("Section '" + sectionName + "' not found in parser");
    }
    
    addSection(sectionName);
    
    for (const auto& [key, param] : section->parameters) {
        addParameter(key, param.value);
    }
    
    return *this;
}

size_t ConfigBuilder::getSectionCount() const {
    return sections_.size();
}

std::vector<std::string> ConfigBuilder::getSectionNames() const {
    std::vector<std::string> names;
    for (const auto& section : sections_) {
        names.push_back(section.name);
    }
    return names;
}

// ============ YAML Support Methods ============

#ifdef IOC_CONFIG_YAML_SUPPORT

bool OopParser::loadFromYaml(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        YAML::Node config = YAML::LoadFile(filepath);
        return loadFromYamlNode(config);
    } catch (const std::exception& e) {
        std::cerr << "YAML load error: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::saveToYaml(const std::string& filepath) const {
    try {
        YAML::Node config;
        
        // Convert sections to YAML
        for (const auto& section : sections_) {
            YAML::Node sectionNode;
            
            for (const auto& param : section.parameters) {
                sectionNode[param.key] = param.value;
            }
            
            config[section.name] = sectionNode;
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << config;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "YAML save error: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::loadFromYamlString(const std::string& yamlString) {
    try {
        YAML::Node config = YAML::Load(yamlString);
        return loadFromYamlNode(config);
    } catch (const std::exception& e) {
        std::cerr << "YAML parse error: " << e.what() << std::endl;
        return false;
    }
}

std::string OopParser::saveToYamlString() const {
    try {
        YAML::Node config;
        
        // Convert sections to YAML
        for (const auto& section : sections_) {
            YAML::Node sectionNode;
            
            for (const auto& param : section.parameters) {
                sectionNode[param.key] = param.value;
            }
            
            config[section.name] = sectionNode;
        }
        
        YAML::Emitter emitter;
        emitter << config;
        return emitter.c_str();
    } catch (const std::exception& e) {
        std::cerr << "YAML format error: " << e.what() << std::endl;
        return "";
    }
}

// Helper method to load from YAML node
bool OopParser::loadFromYamlNode(const YAML::Node& config) {
    try {
        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            std::string sectionName = it->first.as<std::string>();
            const YAML::Node& sectionNode = it->second;
            
            if (sectionNode.IsMap()) {
                for (YAML::const_iterator paramIt = sectionNode.begin(); 
                     paramIt != sectionNode.end(); ++paramIt) {
                    std::string paramKey = paramIt->first.as<std::string>();
                    std::string paramValue = paramIt->second.as<std::string>();
                    
                    setParameter(sectionName, paramKey, paramValue);
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "YAML node load error: " << e.what() << std::endl;
        return false;
    }
}

#else

// Stub implementations when YAML support is disabled
bool OopParser::loadFromYaml(const std::string& filepath) {
    std::cerr << "YAML support not available (yaml-cpp not found)" << std::endl;
    return false;
}

bool OopParser::saveToYaml(const std::string& filepath) const {
    std::cerr << "YAML support not available (yaml-cpp not found)" << std::endl;
    return false;
}

bool OopParser::loadFromYamlString(const std::string& yamlString) {
    std::cerr << "YAML support not available (yaml-cpp not found)" << std::endl;
    return false;
}

std::string OopParser::saveToYamlString() const {
    std::cerr << "YAML support not available (yaml-cpp not found)" << std::endl;
    return "";
}

#endif

// ============ XML Support Implementation ============

#include <cstring>
#include <fstream>

bool OopParser::loadFromXml(const std::string& filepath) {
    if (!isXmlSupported()) {
        std::cerr << "XML support not available (libxml2 not compiled)" << std::endl;
        return false;
    }
    
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open XML file: " << filepath << std::endl;
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return loadFromXmlString(content);
    } catch (const std::exception& e) {
        std::cerr << "Error loading XML: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::saveToXml(const std::string& filepath) const {
    if (!isXmlSupported()) {
        std::cerr << "XML support not available (libxml2 not compiled)" << std::endl;
        return false;
    }
    
    try {
        std::string xml = saveToXmlString();
        if (xml.empty()) {
            return false;
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to create XML file: " << filepath << std::endl;
            return false;
        }
        
        file << xml;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving XML: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::loadFromXmlString(const std::string& xmlString) {
    if (!isXmlSupported()) {
        std::cerr << "XML support not available (libxml2 not compiled)" << std::endl;
        return false;
    }
    
    if (xmlString.empty()) {
        std::cerr << "Empty XML string provided" << std::endl;
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        clear();
        
        // Simple XML parsing without libxml2 dependency
        // This is a robust implementation that uses regex for element parsing
        
        // Helper lambda to extract tag name from opening tag
        auto extract_tag_name = [](const std::string& tag) -> std::string {
            size_t name_end = tag.find_first_of(" />");
            if (name_end != std::string::npos) {
                return tag.substr(0, name_end);
            }
            return tag;
        };
        
        // Helper lambda to extract attributes from tag
        auto extract_attributes = [](const std::string& tag) -> std::map<std::string, std::string> {
            std::map<std::string, std::string> attrs;
            
            // Simple regex pattern for attributes: name="value" or name='value'
            std::regex attr_pattern(R"((\w+)\s*=\s*["\']([^"\']*)["\'])");
            std::smatch match;
            std::string::const_iterator search_start(tag.cbegin());
            
            while (std::regex_search(search_start, tag.cend(), match, attr_pattern)) {
                attrs[match[1].str()] = match[2].str();
                search_start = match.suffix().first;
            }
            
            return attrs;
        };
        
        size_t pos = 0;
        
        // Skip XML declaration
        if (xmlString.find("<?xml") == 0) {
            pos = xmlString.find("?>");
            if (pos != std::string::npos) {
                pos += 2;
            }
        }
        
        // Find all opening tags
        std::regex tag_pattern(R"(<([^/>]+?)(?:\s+([^>]*))?(?:/>|>))");
        std::smatch match;
        std::string remaining = xmlString.substr(pos);
        std::string::const_iterator search_start(remaining.cbegin());
        
        while (std::regex_search(search_start, remaining.cend(), match, tag_pattern)) {
            std::string full_tag = match[0].str();
            std::string tag_content = match[1].str();
            
            // Skip closing tags, XML declaration, and comments
            if (tag_content[0] == '/' || tag_content[0] == '?' || tag_content[0] == '!') {
                search_start = match.suffix().first;
                continue;
            }
            
            // Extract tag name
            std::string tag_name = extract_tag_name(tag_content);
            
            if (tag_name.empty() || tag_name == "config") {
                search_start = match.suffix().first;
                continue;
            }
            
            // Create section
            ConfigSectionData section;
            section.name = tag_name;
            section.type = ConfigSectionData::stringToSectionType(tag_name);
            
            // Extract attributes
            std::string attr_string = match[2].str();
            if (!attr_string.empty()) {
                auto attrs = extract_attributes(attr_string);
                for (const auto& [key, value] : attrs) {
                    ConfigParameter param;
                    param.key = "." + key;
                    param.value = value;
                    param.type = detectType(value);
                    section.parameters[param.key] = param;
                }
            }
            
            // Extract text content if present (between > and </)
            size_t match_end = match.position() + match.length();
            if (full_tag.back() != '/' && full_tag.find("/>") == std::string::npos) {
                size_t content_start = match_end;
                size_t closing_tag_pos = remaining.find("</" + tag_name + ">", content_start);
                
                if (closing_tag_pos != std::string::npos && closing_tag_pos > content_start) {
                    std::string content = remaining.substr(content_start, closing_tag_pos - content_start);
                    
                    // Trim whitespace
                    size_t first = content.find_first_not_of(" \t\n\r");
                    size_t last = content.find_last_not_of(" \t\n\r");
                    
                    if (first != std::string::npos) {
                        content = content.substr(first, last - first + 1);
                        
                        if (!content.empty()) {
                            ConfigParameter param;
                            param.key = "._content";
                            param.value = content;
                            param.type = "string";
                            section.parameters[param.key] = param;
                        }
                    }
                }
            }
            
            if (!section.parameters.empty()) {
                sections_.push_back(section);
            }
            
            search_start = match.suffix().first;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing XML: " << e.what() << std::endl;
        return false;
    }
}

std::string OopParser::saveToXmlString() const {
    if (!isXmlSupported()) {
        return "";
    }
    
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        
        std::ostringstream oss;
        oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        oss << "<config>\n";
        
        for (const auto& section : sections_) {
            oss << "  <" << section.name;
            
            // Output attributes
            for (const auto& [key, param] : section.parameters) {
                if (key != "._content") {
                    // Remove leading dot
                    std::string attr_name = (key.length() > 0 && key[0] == '.') ? key.substr(1) : key;
                    oss << " " << attr_name << "=\"";
                    
                    // Escape XML special characters
                    for (char c : param.value) {
                        switch (c) {
                            case '&': oss << "&amp;"; break;
                            case '<': oss << "&lt;"; break;
                            case '>': oss << "&gt;"; break;
                            case '"': oss << "&quot;"; break;
                            case '\'': oss << "&apos;"; break;
                            default: oss << c;
                        }
                    }
                    oss << "\"";
                }
            }
            
            // Check for content
            auto content_it = section.parameters.find("._content");
            if (content_it != section.parameters.end()) {
                oss << ">";
                // Escape XML special characters in content
                for (char c : content_it->second.value) {
                    switch (c) {
                        case '&': oss << "&amp;"; break;
                        case '<': oss << "&lt;"; break;
                        case '>': oss << "&gt;"; break;
                        default: oss << c;
                    }
                }
                oss << "</" << section.name << ">\n";
            } else {
                oss << " />\n";
            }
        }
        
        oss << "</config>\n";
        return oss.str();
    } catch (const std::exception& e) {
        std::cerr << "Error generating XML: " << e.what() << std::endl;
        return "";
    }
}

bool OopParser::isXmlSupported() {
    // XML support is always available with our built-in parser
    return true;
}

// ============ CSV Support Implementation ============

char OopParser::detectCsvDelimiter(const std::string& csvContent) {
    if (csvContent.empty()) {
        return ',';
    }
    
    // Count occurrences of common delimiters in first line
    size_t first_line_end = csvContent.find('\n');
    if (first_line_end == std::string::npos) {
        first_line_end = csvContent.length();
    }
    
    std::string first_line = csvContent.substr(0, first_line_end);
    
    size_t comma_count = 0, semicolon_count = 0, tab_count = 0;
    bool in_quotes = false;
    
    for (char c : first_line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (c == ',') comma_count++;
            else if (c == ';') semicolon_count++;
            else if (c == '\t') tab_count++;
        }
    }
    
    // Return delimiter with highest count
    if (semicolon_count > comma_count && semicolon_count > tab_count) {
        return ';';
    } else if (tab_count > comma_count) {
        return '\t';
    }
    return ',';
}

bool OopParser::loadFromCsv(const std::string& filepath, bool hasHeader) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open CSV file: " << filepath << std::endl;
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        return loadFromCsvString(content, hasHeader);
    } catch (const std::exception& e) {
        std::cerr << "Error loading CSV: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::saveToCsv(const std::string& filepath, bool withHeader) const {
    try {
        std::string csv = saveToCsvString(withHeader);
        if (csv.empty()) {
            return false;
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to create CSV file: " << filepath << std::endl;
            return false;
        }
        
        file << csv;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving CSV: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::loadFromCsvString(const std::string& csvString, bool hasHeader) {
    if (csvString.empty()) {
        std::cerr << "Empty CSV string provided" << std::endl;
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        clear();
        
        // Detect delimiter
        char delimiter = detectCsvDelimiter(csvString);
        
        // Split into lines
        std::vector<std::vector<std::string>> rows;
        std::istringstream stream(csvString);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            
            std::vector<std::string> row;
            std::string field;
            bool in_quotes = false;
            
            for (size_t i = 0; i < line.length(); i++) {
                char c = line[i];
                
                if (c == '"') {
                    in_quotes = !in_quotes;
                    // Include quotes in field if not at boundaries
                    if (i > 0 && i < line.length() - 1) {
                        field += c;
                    }
                } else if (c == delimiter && !in_quotes) {
                    row.push_back(field);
                    field.clear();
                } else {
                    field += c;
                }
            }
            row.push_back(field);
            rows.push_back(row);
        }
        
        if (rows.empty()) {
            return true; // Empty CSV
        }
        
        // Process header
        std::vector<std::string> headers;
        size_t data_start = 0;
        
        if (hasHeader && !rows.empty()) {
            headers = rows[0];
            data_start = 1;
            
            // Clean headers
            for (auto& h : headers) {
                // Trim whitespace
                size_t start = h.find_first_not_of(" \t");
                size_t end = h.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    h = h.substr(start, end - start + 1);
                }
            }
        }
        
        // Process data rows
        for (size_t i = data_start; i < rows.size(); i++) {
            const auto& row = rows[i];
            if (row.empty()) continue;
            
            // First column is section name
            std::string section_name = row[0];
            
            // Trim whitespace
            size_t start = section_name.find_first_not_of(" \t");
            size_t end = section_name.find_last_not_of(" \t");
            if (start != std::string::npos) {
                section_name = section_name.substr(start, end - start + 1);
            }
            
            if (section_name.empty()) continue;
            
            ConfigSectionData section;
            section.name = section_name;
            section.type = ConfigSectionData::stringToSectionType(section_name);
            
            // Add parameters for remaining columns
            for (size_t j = 1; j < row.size(); j++) {
                if (j >= headers.size() || headers[j].empty()) {
                    continue;
                }
                
                ConfigParameter param;
                param.key = "." + headers[j];
                param.value = row[j];
                
                // Trim whitespace from value
                size_t v_start = param.value.find_first_not_of(" \t");
                size_t v_end = param.value.find_last_not_of(" \t");
                if (v_start != std::string::npos) {
                    param.value = param.value.substr(v_start, v_end - v_start + 1);
                }
                
                param.type = detectType(param.value);
                section.parameters[param.key] = param;
            }
            
            if (!section.parameters.empty()) {
                sections_.push_back(section);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing CSV: " << e.what() << std::endl;
        return false;
    }
}

std::string OopParser::saveToCsvString(bool withHeader) const {
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        
        if (sections_.empty()) {
            return "";
        }
        
        std::ostringstream oss;
        
        // Collect all unique parameter keys
        std::set<std::string> all_keys;
        for (const auto& section : sections_) {
            for (const auto& [key, param] : section.parameters) {
                all_keys.insert(key);
            }
        }
        
        std::vector<std::string> sorted_keys(all_keys.begin(), all_keys.end());
        
        // Write header if requested
        if (withHeader) {
            oss << "Section";
            for (const auto& key : sorted_keys) {
                oss << ",";
                // Remove leading dot for header
                std::string header_name = (key.length() > 0 && key[0] == '.') ? key.substr(1) : key;
                oss << "\"" << header_name << "\"";
            }
            oss << "\n";
        }
        
        // Write data rows
        for (const auto& section : sections_) {
            oss << section.name;
            
            for (const auto& key : sorted_keys) {
                oss << ",";
                
                auto it = section.parameters.find(key);
                if (it != section.parameters.end()) {
                    const auto& value = it->second.value;
                    
                    // Quote if contains comma, quotes, or newlines
                    if (value.find(',') != std::string::npos ||
                        value.find('"') != std::string::npos ||
                        value.find('\n') != std::string::npos) {
                        
                        oss << "\"";
                        for (char c : value) {
                            if (c == '"') {
                                oss << "\"\""; // Escape quotes
                            } else {
                                oss << c;
                            }
                        }
                        oss << "\"";
                    } else {
                        oss << value;
                    }
                }
            }
            
            oss << "\n";
        }
        
        return oss.str();
    } catch (const std::exception& e) {
        std::cerr << "Error generating CSV: " << e.what() << std::endl;
        return "";
    }
}

#ifdef IOC_CONFIG_TOML_SUPPORT

bool OopParser::loadFromToml(const std::string& filepath) {
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        
        clear();
        const auto data = toml::parse(filepath);
        
        for (const auto& [section_name, section_value] : data) {
            ConfigSectionData section;
            section.name = section_name;
            section.type = ConfigSectionData::stringToSectionType(section_name);
            
            if (section_value.is_table()) {
                for (const auto& [key, value] : *section_value.as_table()) {
                    ConfigParameter param;
                    param.key = "." + key;
                    
                    if (value.is_string()) {
                        param.value = value.as_string();
                        param.type = "string";
                    } else if (value.is_integer()) {
                        param.value = std::to_string(value.as_integer());
                        param.type = "int";
                    } else if (value.is_floating()) {
                        param.value = std::to_string(value.as_floating());
                        param.type = "float";
                    } else if (value.is_boolean()) {
                        param.value = value.as_boolean() ? "true" : "false";
                        param.type = "bool";
                    }
                    
                    section.parameters[param.key] = param;
                }
            }
            
            if (!section.parameters.empty()) {
                sections_.push_back(section);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = "TOML parsing error: " + std::string(e.what());
        std::cerr << lastError_ << "\n";
        return false;
    }
}

bool OopParser::saveToToml(const std::string& filepath) const {
    try {
        toml::table root;
        
        for (const auto& section : sections_) {
            toml::table section_table;
            
            for (const auto& [key, param] : section.parameters) {
                std::string clean_key = key;
                if (clean_key[0] == '.') {
                    clean_key = clean_key.substr(1);
                }
                
                if (param.type == "int") {
                    section_table.insert(clean_key, std::stoll(param.value));
                } else if (param.type == "float") {
                    section_table.insert(clean_key, std::stod(param.value));
                } else if (param.type == "bool") {
                    section_table.insert(clean_key, param.value == "true");
                } else {
                    section_table.insert(clean_key, param.value);
                }
            }
            
            root.insert(section.name, section_table);
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            lastError_ = "Cannot open file for writing: " + filepath;
            return false;
        }
        
        file << root;
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = "TOML export error: " + std::string(e.what());
        std::cerr << lastError_ << "\n";
        return false;
    }
}

bool OopParser::loadFromTomlString(const std::string& tomlString) {
    try {
        std::lock_guard<std::mutex> lock(sectionsMutex_);
        
        clear();
        std::istringstream stream(tomlString);
        const auto data = toml::parse(stream);
        
        for (const auto& [section_name, section_value] : data) {
            ConfigSectionData section;
            section.name = section_name;
            section.type = ConfigSectionData::stringToSectionType(section_name);
            
            if (section_value.is_table()) {
                for (const auto& [key, value] : *section_value.as_table()) {
                    ConfigParameter param;
                    param.key = "." + key;
                    
                    if (value.is_string()) {
                        param.value = value.as_string();
                        param.type = "string";
                    } else if (value.is_integer()) {
                        param.value = std::to_string(value.as_integer());
                        param.type = "int";
                    } else if (value.is_floating()) {
                        param.value = std::to_string(value.as_floating());
                        param.type = "float";
                    } else if (value.is_boolean()) {
                        param.value = value.as_boolean() ? "true" : "false";
                        param.type = "bool";
                    }
                    
                    section.parameters[param.key] = param;
                }
            }
            
            if (!section.parameters.empty()) {
                sections_.push_back(section);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = "TOML parsing error: " + std::string(e.what());
        std::cerr << lastError_ << "\n";
        return false;
    }
}

std::string OopParser::saveToTomlString() const {
    try {
        toml::table root;
        
        for (const auto& section : sections_) {
            toml::table section_table;
            
            for (const auto& [key, param] : section.parameters) {
                std::string clean_key = key;
                if (clean_key[0] == '.') {
                    clean_key = clean_key.substr(1);
                }
                
                if (param.type == "int") {
                    section_table.insert(clean_key, std::stoll(param.value));
                } else if (param.type == "float") {
                    section_table.insert(clean_key, std::stod(param.value));
                } else if (param.type == "bool") {
                    section_table.insert(clean_key, param.value == "true");
                } else {
                    section_table.insert(clean_key, param.value);
                }
            }
            
            root.insert(section.name, section_table);
        }
        
        std::ostringstream oss;
        oss << root;
        return oss.str();
    } catch (const std::exception& e) {
        return "";
    }
}

#else

// Stub implementations when TOML support is disabled
bool OopParser::loadFromToml(const std::string&) {
    lastError_ = "TOML support not available (toml11 not found)";
    std::cerr << lastError_ << "\n";
    return false;
}

bool OopParser::saveToToml(const std::string&) const {
    std::cerr << "TOML support not available (toml11 not found)\n";
    return false;
}

bool OopParser::loadFromTomlString(const std::string&) {
    lastError_ = "TOML support not available (toml11 not found)";
    std::cerr << lastError_ << "\n";
    return false;
}

std::string OopParser::saveToTomlString() const {
    return "";
}

#endif

// ============ Merge & Diff Implementation ============

bool OopParser::merge(const OopParser& other, MergeStrategy strategy) {
    if (strategy == MergeStrategy::CUSTOM) {
        lastError_ = "CUSTOM strategy requires resolver callback";
        return false;
    }

    // Reset statistics
    mergeStats_ = MergeStats();

    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& other_section : other.sections_) {
        auto it = std::find_if(sections_.begin(), sections_.end(),
            [&](const ConfigSectionData& s) { return s.name == other_section.name; });

        if (it == sections_.end()) {
            // Section doesn't exist - add it
            sections_.push_back(other_section);
            mergeStats_.sections_added++;
        } else {
            // Section exists - merge parameters based on strategy
            if (strategy == MergeStrategy::REPLACE || strategy == MergeStrategy::DEEP_MERGE) {
                for (const auto& [key, param] : other_section.parameters) {
                    if (it->parameters.find(key) != it->parameters.end()) {
                        if (it->parameters[key].value != param.value) {
                            it->parameters[key] = param;
                            mergeStats_.parameters_modified++;
                        }
                    } else {
                        it->parameters[key] = param;
                        mergeStats_.parameters_added++;
                    }
                }
                mergeStats_.sections_updated++;
            } else if (strategy == MergeStrategy::APPEND) {
                // Only add new parameters, don't replace existing
                for (const auto& [key, param] : other_section.parameters) {
                    if (it->parameters.find(key) == it->parameters.end()) {
                        it->parameters[key] = param;
                        mergeStats_.parameters_added++;
                    }
                }
            }
        }
    }

    return true;
}

bool OopParser::mergeWithResolver(const OopParser& other,
                                  std::function<MergeConflict(const MergeConflict&)> resolver) {
    if (!resolver) {
        lastError_ = "Resolver callback cannot be null";
        return false;
    }

    mergeStats_ = MergeStats();
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& other_section : other.sections_) {
        auto it = std::find_if(sections_.begin(), sections_.end(),
            [&](const ConfigSectionData& s) { return s.name == other_section.name; });

        if (it == sections_.end()) {
            sections_.push_back(other_section);
            mergeStats_.sections_added++;
        } else {
            for (const auto& [key, param] : other_section.parameters) {
                auto existing = it->parameters.find(key);
                if (existing != it->parameters.end() && existing->second.value != param.value) {
                    // Conflict detected - use resolver
                    MergeConflict conflict;
                    conflict.section = other_section.name;
                    conflict.key = key;
                    conflict.existingValue = existing->second.value;
                    conflict.incomingValue = param.value;
                    conflict.resolvedValue = param.value;
                    conflict.resolved = false;

                    auto resolved = resolver(conflict);
                    if (resolved.resolved) {
                        existing->second.value = resolved.resolvedValue;
                        mergeStats_.parameters_modified++;
                    } else {
                        mergeStats_.conflicts++;
                        mergeStats_.conflict_keys.push_back(key);
                    }
                } else if (existing == it->parameters.end()) {
                    it->parameters[key] = param;
                    mergeStats_.parameters_added++;
                }
            }
            mergeStats_.sections_updated++;
        }
    }

    return mergeStats_.conflicts == 0;
}

const MergeStats& OopParser::getLastMergeStats() const {
    return mergeStats_;
}

std::vector<DiffEntry> OopParser::diff(const OopParser& other) const {
    std::vector<DiffEntry> diffs;
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    // Check for sections in this but not in other (REMOVED)
    for (const auto& section : sections_) {
        auto other_section = std::find_if(other.sections_.begin(), other.sections_.end(),
            [&](const ConfigSectionData& s) { return s.name == section.name; });

        if (other_section == other.sections_.end()) {
            for (const auto& [key, param] : section.parameters) {
                DiffEntry entry;
                entry.type = DiffEntry::REMOVED;
                entry.section = section.name;
                entry.key = key;
                entry.oldValue = param.value;
                entry.oldType = param.type;
                diffs.push_back(entry);
            }
        } else {
            // Check individual parameters
            for (const auto& [key, param] : section.parameters) {
                auto other_param = other_section->parameters.find(key);
                if (other_param == other_section->parameters.end()) {
                    // Parameter removed
                    DiffEntry entry;
                    entry.type = DiffEntry::REMOVED;
                    entry.section = section.name;
                    entry.key = key;
                    entry.oldValue = param.value;
                    entry.oldType = param.type;
                    diffs.push_back(entry);
                } else if (other_param->second.value != param.value) {
                    // Parameter modified
                    DiffEntry entry;
                    entry.type = DiffEntry::MODIFIED;
                    entry.section = section.name;
                    entry.key = key;
                    entry.oldValue = param.value;
                    entry.newValue = other_param->second.value;
                    entry.oldType = param.type;
                    entry.newType = other_param->second.type;
                    diffs.push_back(entry);
                } else {
                    // Unchanged
                    DiffEntry entry;
                    entry.type = DiffEntry::UNCHANGED;
                    entry.section = section.name;
                    entry.key = key;
                    entry.oldValue = param.value;
                    entry.oldType = param.type;
                    diffs.push_back(entry);
                }
            }

            // Check for added parameters
            for (const auto& [key, param] : other_section->parameters) {
                if (section.parameters.find(key) == section.parameters.end()) {
                    DiffEntry entry;
                    entry.type = DiffEntry::ADDED;
                    entry.section = section.name;
                    entry.key = key;
                    entry.newValue = param.value;
                    entry.newType = param.type;
                    diffs.push_back(entry);
                }
            }
        }
    }

    // Check for sections in other but not in this (ADDED)
    for (const auto& other_section : other.sections_) {
        auto section = std::find_if(sections_.begin(), sections_.end(),
            [&](const ConfigSectionData& s) { return s.name == other_section.name; });

        if (section == sections_.end()) {
            for (const auto& [key, param] : other_section.parameters) {
                DiffEntry entry;
                entry.type = DiffEntry::ADDED;
                entry.section = other_section.name;
                entry.key = key;
                entry.newValue = param.value;
                entry.newType = param.type;
                diffs.push_back(entry);
            }
        }
    }

    return diffs;
}

std::string OopParser::diffReport(const OopParser& other, bool onlyChanges) const {
    auto diffs = diff(other);
    std::ostringstream oss;

    oss << "=== Configuration Diff Report ===\n";
    
    size_t added = 0, removed = 0, modified = 0, unchanged = 0;

    for (const auto& entry : diffs) {
        if (onlyChanges && entry.type == DiffEntry::UNCHANGED) {
            unchanged++;
            continue;
        }

        oss << entry.toString() << "\n";

        if (entry.type == DiffEntry::ADDED) added++;
        else if (entry.type == DiffEntry::REMOVED) removed++;
        else if (entry.type == DiffEntry::MODIFIED) modified++;
        else unchanged++;
    }

    oss << "\n--- Summary ---\n";
    oss << "Added: " << added << "\n";
    oss << "Removed: " << removed << "\n";
    oss << "Modified: " << modified << "\n";
    if (!onlyChanges) oss << "Unchanged: " << unchanged << "\n";

    return oss.str();
}

nlohmann::json OopParser::diffAsJson(const OopParser& other) const {
    auto diffs = diff(other);
    json result = json::array();

    for (const auto& entry : diffs) {
        json diff_obj;
        diff_obj["type"] = [](DiffEntry::Type t) {
            switch (t) {
                case DiffEntry::ADDED: return "added";
                case DiffEntry::REMOVED: return "removed";
                case DiffEntry::MODIFIED: return "modified";
                case DiffEntry::UNCHANGED: return "unchanged";
                default: return "unknown";
            }
        }(entry.type);
        diff_obj["section"] = entry.section;
        diff_obj["key"] = entry.key;
        if (!entry.oldValue.empty()) diff_obj["old_value"] = entry.oldValue;
        if (!entry.newValue.empty()) diff_obj["new_value"] = entry.newValue;
        if (!entry.oldType.empty()) diff_obj["old_type"] = entry.oldType;
        if (!entry.newType.empty()) diff_obj["new_type"] = entry.newType;
        
        result.push_back(diff_obj);
    }

    return result;
}

// ============ Cloning & Copying Implementation ============

std::unique_ptr<OopParser> OopParser::clone() const {
    auto cloned = std::make_unique<OopParser>();
    cloned->copyFrom(*this);
    return cloned;
}

OopParser& OopParser::copyFrom(const OopParser& other) {
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    sections_ = other.sections_;
    lastError_ = other.lastError_;
    if (other.schema_) {
        schema_ = std::make_unique<ConfigSchema>(*other.schema_);
    }
    mergeStats_ = other.mergeStats_;

    return *this;
}

bool OopParser::isEmpty() const {
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    return sections_.empty();
}

// ============ Query & Filter Implementation ============

std::vector<ConfigParameter> OopParser::getParametersWhere(
    std::function<bool(const ConfigParameter&)> predicate) const {
    std::vector<ConfigParameter> results;
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& section : sections_) {
        for (const auto& [key, param] : section.parameters) {
            if (predicate(param)) {
                results.push_back(param);
            }
        }
    }

    return results;
}

std::vector<ConfigSectionData> OopParser::getSectionsWhere(
    std::function<bool(const ConfigSectionData&)> predicate) const {
    std::vector<ConfigSectionData> results;
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& section : sections_) {
        if (predicate(section)) {
            results.push_back(section);
        }
    }

    return results;
}

ConfigParameter* OopParser::findWhere(std::function<bool(const ConfigParameter&)> predicate) {
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (auto& section : sections_) {
        for (auto& [key, param] : section.parameters) {
            if (predicate(param)) {
                return &param;
            }
        }
    }

    return nullptr;
}

const ConfigParameter* OopParser::findWhere(std::function<bool(const ConfigParameter&)> predicate) const {
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& section : sections_) {
        for (const auto& [key, param] : section.parameters) {
            if (predicate(param)) {
                return &param;
            }
        }
    }

    return nullptr;
}

std::vector<ConfigParameter> OopParser::getParametersByKeyPattern(const std::string& pattern) const {
    std::vector<ConfigParameter> results;
    try {
        std::regex key_regex(pattern, std::regex::icase);
        std::lock_guard<std::mutex> lock(sectionsMutex_);

        for (const auto& section : sections_) {
            for (const auto& [key, param] : section.parameters) {
                if (std::regex_search(key, key_regex)) {
                    results.push_back(param);
                }
            }
        }
    } catch (const std::regex_error& e) {
        lastError_ = std::string("Invalid regex pattern: ") + e.what();
    }

    return results;
}

std::vector<ConfigParameter> OopParser::getParametersByValuePattern(const std::string& pattern) const {
    std::vector<ConfigParameter> results;
    try {
        std::regex value_regex(pattern, std::regex::icase);
        std::lock_guard<std::mutex> lock(sectionsMutex_);

        for (const auto& section : sections_) {
            for (const auto& [key, param] : section.parameters) {
                if (std::regex_search(param.value, value_regex)) {
                    results.push_back(param);
                }
            }
        }
    } catch (const std::regex_error& e) {
        lastError_ = std::string("Invalid regex pattern: ") + e.what();
    }

    return results;
}

std::vector<ConfigParameter> OopParser::getParametersByType(const std::string& type) const {
    std::vector<ConfigParameter> results;
    std::lock_guard<std::mutex> lock(sectionsMutex_);

    for (const auto& section : sections_) {
        for (const auto& [key, param] : section.parameters) {
            if (param.type == type) {
                results.push_back(param);
            }
        }
    }

    return results;
}

// ============ Path-Based Access Implementation (RFC 6901) ============

std::vector<std::string> OopParser::parsePath(const std::string& path) {
    std::vector<std::string> components;
    
    if (path.empty() || path == "/") {
        return components;  // Root path
    }
    
    if (path[0] != '/') {
        // Invalid path, must start with /
        return components;
    }
    
    size_t start = 1;
    size_t pos = 1;
    
    while (pos <= path.length()) {
        if (pos == path.length() || path[pos] == '/') {
            if (pos > start) {
                std::string token = path.substr(start, pos - start);
                components.push_back(unescapePathToken(token));
            }
            start = pos + 1;
        }
        pos++;
    }
    
    return components;
}

std::string OopParser::escapePathToken(const std::string& token) {
    std::string escaped;
    for (char c : token) {
        if (c == '~') {
            escaped += "~0";
        } else if (c == '/') {
            escaped += "~1";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

std::string OopParser::unescapePathToken(const std::string& token) {
    std::string unescaped;
    size_t i = 0;
    
    while (i < token.length()) {
        if (token[i] == '~' && i + 1 < token.length()) {
            if (token[i + 1] == '0') {
                unescaped += '~';
                i += 2;
            } else if (token[i + 1] == '1') {
                unescaped += '/';
                i += 2;
            } else {
                unescaped += token[i];
                i++;
            }
        } else {
            unescaped += token[i];
            i++;
        }
    }
    
    return unescaped;
}

std::string OopParser::getValueByPath(const std::string& path) const {
    auto components = parsePath(path);
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    // Root path
    if (components.empty()) {
        json root_json = json::object();
        for (const auto& section : sections_) {
            json section_obj = json::object();
            for (const auto& [key, param] : section.parameters) {
                section_obj[key] = param.value;
            }
            root_json[section.name] = section_obj;
        }
        return root_json.dump();
    }
    
    // Find section
    if (components.size() >= 1) {
        auto section_it = std::find_if(sections_.begin(), sections_.end(),
            [&](const ConfigSectionData& s) { return s.name == components[0]; });
        
        if (section_it == sections_.end()) {
            return "";  // Section not found
        }
        
        // If only section requested
        if (components.size() == 1) {
            json section_obj = json::object();
            for (const auto& [key, param] : section_it->parameters) {
                section_obj[key] = param.value;
            }
            return section_obj.dump();
        }
        
        // Find parameter
        if (components.size() >= 2) {
            auto param_it = section_it->parameters.find(components[1]);
            if (param_it != section_it->parameters.end()) {
                return param_it->second.value;
            }
        }
    }
    
    return "";
}

bool OopParser::setValueByPath(const std::string& path, const std::string& value) {
    auto components = parsePath(path);
    
    if (components.empty() || components.size() < 2) {
        lastError_ = "Path must have at least section and key";
        return false;
    }
    
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    // Find or create section
    auto section_it = std::find_if(sections_.begin(), sections_.end(),
        [&](const ConfigSectionData& s) { return s.name == components[0]; });
    
    if (section_it == sections_.end()) {
        // Create new section
        ConfigSectionData new_section;
        new_section.name = components[0];
        new_section.type = ConfigSectionData::stringToSectionType(components[0]);
        sections_.push_back(new_section);
        section_it = sections_.end() - 1;
    }
    
    // Set parameter
    ConfigParameter param;
    param.key = components[1];
    param.value = value;
    param.type = detectType(value);
    
    section_it->parameters[components[1]] = param;
    
    return true;
}

bool OopParser::hasPath(const std::string& path) const {
    return !getValueByPath(path).empty();
}

bool OopParser::deleteByPath(const std::string& path) {
    auto components = parsePath(path);
    
    if (components.empty()) {
        lastError_ = "Cannot delete root path";
        return false;
    }
    
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    // Find section
    auto section_it = std::find_if(sections_.begin(), sections_.end(),
        [&](const ConfigSectionData& s) { return s.name == components[0]; });
    
    if (section_it == sections_.end()) {
        lastError_ = "Section not found: " + components[0];
        return false;
    }
    
    if (components.size() == 1) {
        // Delete entire section
        sections_.erase(section_it);
        return true;
    }
    
    if (components.size() >= 2) {
        // Delete parameter
        auto param_it = section_it->parameters.find(components[1]);
        if (param_it != section_it->parameters.end()) {
            section_it->parameters.erase(param_it);
            return true;
        }
    }
    
    lastError_ = "Path not found: " + path;
    return false;
}

std::vector<std::string> OopParser::getAllPaths() const {
    std::vector<std::string> paths;
    std::lock_guard<std::mutex> lock(sectionsMutex_);
    
    for (const auto& section : sections_) {
        // Add section path
        paths.push_back("/" + escapePathToken(section.name));
        
        // Add parameter paths
        for (const auto& [key, param] : section.parameters) {
            std::string param_path = "/" + escapePathToken(section.name) + 
                                     "/" + escapePathToken(key);
            paths.push_back(param_path);
        }
    }
    
    return paths;
}

// ============ Utility Functions ============

bool convertOopToJson(const std::string& oopFilepath, 
                      const std::string& jsonFilepath) {
    OopParser parser;
    if (!parser.loadFromOop(oopFilepath)) {
        return false;
    }
    return parser.saveToJson(jsonFilepath);
}

bool convertJsonToOop(const std::string& jsonFilepath, 
                      const std::string& oopFilepath) {
    OopParser parser;
    if (!parser.loadFromJson(jsonFilepath)) {
        return false;
    }
    return parser.saveToOop(oopFilepath);
}

std::string getLibraryVersion() {
    return "1.0.0";
}

// ============ BatchProcessor Implementation ============

BatchProcessor::BatchProcessor() {
    clearStats();
}

BatchStats BatchProcessor::validateAll(const std::vector<std::string>& filepaths) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    BatchStats stats;
    stats.total_files = filepaths.size();
    
    for (const auto& filepath : filepaths) {
        OopParser parser;
        if (!parser.loadFromOop(filepath)) {
            stats.failed_operations++;
            stats.failed_files.push_back(filepath);
            stats.error_messages.push_back("Failed to load: " + filepath);
            continue;
        }
        
        // Check for empty configuration
        if (parser.isEmpty()) {
            stats.failed_operations++;
            stats.failed_files.push_back(filepath);
            stats.error_messages.push_back("Empty configuration: " + filepath);
            continue;
        }
        
        stats.successful_operations++;
    }
    
    lastStats_ = stats;
    return stats;
}

BatchStats BatchProcessor::convertAll(const std::vector<std::string>& sourceFiles,
                                     const std::string& sourceFormat,
                                     const std::string& targetFormat,
                                     const std::string& outputDirectory) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    BatchStats stats;
    stats.total_files = sourceFiles.size();
    
    for (const auto& sourcePath : sourceFiles) {
        try {
            OopParser parser;
            
            // Load from source format
            if (!loadConfigByFormat(parser, sourcePath, sourceFormat)) {
                stats.failed_operations++;
                stats.failed_files.push_back(sourcePath);
                stats.error_messages.push_back("Failed to load " + sourceFormat + ": " + sourcePath);
                continue;
            }
            
            // Determine output path
            std::string outputPath = sourcePath;
            if (!outputDirectory.empty()) {
                size_t lastSlash = sourcePath.find_last_of("/\\");
                std::string filename = (lastSlash != std::string::npos) 
                    ? sourcePath.substr(lastSlash + 1) 
                    : sourcePath;
                outputPath = outputDirectory + "/" + getOutputFilename(filename, targetFormat);
            } else {
                outputPath = getOutputFilename(sourcePath, targetFormat);
            }
            
            // Save to target format
            if (!saveConfigByFormat(parser, outputPath, targetFormat)) {
                stats.failed_operations++;
                stats.failed_files.push_back(sourcePath);
                stats.error_messages.push_back("Failed to save " + targetFormat + ": " + outputPath);
                continue;
            }
            
            stats.successful_operations++;
        } catch (const std::exception& e) {
            stats.failed_operations++;
            stats.failed_files.push_back(sourcePath);
            stats.error_messages.push_back("Exception: " + std::string(e.what()));
        }
    }
    
    lastStats_ = stats;
    return stats;
}

BatchStats BatchProcessor::mergeAll(const std::vector<std::string>& filepaths,
                                   const std::string& outputFile,
                                   MergeStrategy strategy) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    BatchStats stats;
    stats.total_files = filepaths.size();
    
    if (filepaths.empty()) {
        stats.failed_operations = 1;
        stats.error_messages.push_back("No files provided for merge");
        lastStats_ = stats;
        return stats;
    }
    
    try {
        // Load base configuration
        OopParser baseConfig;
        if (!baseConfig.loadFromOop(filepaths[0])) {
            stats.failed_operations++;
            stats.failed_files.push_back(filepaths[0]);
            stats.error_messages.push_back("Failed to load base config: " + filepaths[0]);
            lastStats_ = stats;
            return stats;
        }
        
        stats.successful_operations++;
        
        // Merge remaining files
        for (size_t i = 1; i < filepaths.size(); ++i) {
            OopParser incomingConfig;
            if (!incomingConfig.loadFromOop(filepaths[i])) {
                stats.failed_operations++;
                stats.failed_files.push_back(filepaths[i]);
                stats.error_messages.push_back("Failed to load config: " + filepaths[i]);
                continue;
            }
            
            if (!baseConfig.merge(incomingConfig, strategy)) {
                stats.failed_operations++;
                stats.failed_files.push_back(filepaths[i]);
                stats.error_messages.push_back("Merge failed for: " + filepaths[i]);
                continue;
            }
            
            stats.successful_operations++;
        }
        
        // Save merged configuration
        if (!baseConfig.saveToOop(outputFile)) {
            stats.failed_operations++;
            stats.failed_files.push_back(outputFile);
            stats.error_messages.push_back("Failed to save merged config: " + outputFile);
            lastStats_ = stats;
            return stats;
        }
        
    } catch (const std::exception& e) {
        stats.failed_operations++;
        stats.error_messages.push_back("Exception during merge: " + std::string(e.what()));
    }
    
    lastStats_ = stats;
    return stats;
}

BatchStats BatchProcessor::getLastStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return lastStats_;
}

void BatchProcessor::clearStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    lastStats_.total_files = 0;
    lastStats_.successful_operations = 0;
    lastStats_.failed_operations = 0;
    lastStats_.failed_files.clear();
    lastStats_.error_messages.clear();
}

bool BatchProcessor::loadConfigByFormat(OopParser& config, const std::string& filepath,
                                       const std::string& format) {
    std::string fmt = format;
    
    // Convert to lowercase
    std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
    
    if (fmt == "oop" || fmt == "txt") {
        return config.loadFromOop(filepath);
    } else if (fmt == "json") {
        return config.loadFromJson(filepath);
    } else if (fmt == "xml") {
        return config.loadFromXml(filepath);
    } else if (fmt == "csv") {
        return config.loadFromCsv(filepath, true);
    } else if (fmt == "yaml" || fmt == "yml") {
        return config.loadFromYaml(filepath);
    } else if (fmt == "toml") {
        return config.loadFromToml(filepath);
    }
    
    std::cerr << "Unknown source format: " << format << std::endl;
    return false;
}

bool BatchProcessor::saveConfigByFormat(const OopParser& config, const std::string& filepath,
                                       const std::string& format) {
    std::string fmt = format;
    
    // Convert to lowercase
    std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
    
    if (fmt == "oop" || fmt == "txt") {
        return const_cast<OopParser&>(config).saveToOop(filepath);
    } else if (fmt == "json") {
        return const_cast<OopParser&>(config).saveToJson(filepath);
    } else if (fmt == "xml") {
        return const_cast<OopParser&>(config).saveToXml(filepath);
    } else if (fmt == "csv") {
        return const_cast<OopParser&>(config).saveToCsv(filepath, true);
    } else if (fmt == "yaml" || fmt == "yml") {
        return const_cast<OopParser&>(config).saveToYaml(filepath);
    } else if (fmt == "toml") {
        return const_cast<OopParser&>(config).saveToToml(filepath);
    }
    
    std::cerr << "Unknown target format: " << format << std::endl;
    return false;
}

std::string BatchProcessor::getOutputFilename(const std::string& sourcePath,
                                             const std::string& targetExtension) {
    // Remove existing extension
    size_t dotPos = sourcePath.find_last_of('.');
    std::string baseName = (dotPos != std::string::npos) 
        ? sourcePath.substr(0, dotPos) 
        : sourcePath;
    
    // Map format to extension
    std::string ext = targetExtension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    std::string extension;
    if (ext == "oop" || ext == "txt") {
        extension = ".oop";
    } else if (ext == "json") {
        extension = ".json";
    } else if (ext == "xml") {
        extension = ".xml";
    } else if (ext == "csv") {
        extension = ".csv";
    } else if (ext == "yaml" || ext == "yml") {
        extension = ".yaml";
    } else if (ext == "toml") {
        extension = ".toml";
    } else {
        extension = "." + ext;
    }
    
    return baseName + extension;
}

// ============ Streaming I/O Implementation ============

bool OopParser::loadFromStream(std::istream& input) {
    if (!input.good()) {
        std::cerr << "Input stream is not in good state" << std::endl;
        return false;
    }
    
    std::string currentSection;
    std::string line;
    
    try {
        while (std::getline(input, line)) {
            // Trim leading/trailing whitespace
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                continue;  // Empty line
            }
            line = line.substr(start);
            
            // Trim trailing whitespace
            size_t end = line.find_last_not_of(" \t\r\n");
            if (end != std::string::npos) {
                line = line.substr(0, end + 1);
            }
            
            // Skip comments
            if (line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
                
                // Trim section name
                size_t sec_start = currentSection.find_first_not_of(" \t");
                size_t sec_end = currentSection.find_last_not_of(" \t");
                if (sec_start != std::string::npos && sec_end != std::string::npos) {
                    currentSection = currentSection.substr(sec_start, sec_end - sec_start + 1);
                }
                continue;
            }
            
            // Parse key-value pair
            size_t delimPos = line.find('=');
            if (delimPos != std::string::npos && !currentSection.empty()) {
                std::string key = line.substr(0, delimPos);
                std::string value = line.substr(delimPos + 1);
                
                // Trim key
                size_t key_start = key.find_first_not_of(" \t");
                size_t key_end = key.find_last_not_of(" \t");
                if (key_start != std::string::npos) {
                    key = key.substr(key_start, key_end - key_start + 1);
                }
                
                // Trim value
                size_t val_start = value.find_first_not_of(" \t");
                size_t val_end = value.find_last_not_of(" \t\r\n");
                if (val_start != std::string::npos && val_end != std::string::npos) {
                    value = value.substr(val_start, val_end - val_start + 1);
                }
                
                // Add to configuration
                setParameter(currentSection, key, value);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading from stream: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::saveToStream(std::ostream& output) const {
    if (!output.good()) {
        std::cerr << "Output stream is not in good state" << std::endl;
        return false;
    }
    
    try {
        auto sections = getAllSections();
        
        for (size_t i = 0; i < sections.size(); ++i) {
            const auto& section = sections[i];
            
            // Write section header
            output << "[" << section.name << "]\n";
            
            // Write parameters
            for (const auto& [key, param] : section.parameters) {
                output << key << " = " << param.value << "\n";
            }
            
            // Add blank line between sections (except for last)
            if (i < sections.size() - 1) {
                output << "\n";
            }
        }
        
        output.flush();
        return output.good();
    } catch (const std::exception& e) {
        std::cerr << "Error saving to stream: " << e.what() << std::endl;
        return false;
    }
}

bool OopParser::loadFromPipe(int fd) {
    #ifdef _WIN32
    std::cerr << "Pipe operations not supported on Windows" << std::endl;
    return false;
    #else
    // Use fdopen to convert file descriptor to stream
    FILE* file = fdopen(fd, "r");
    if (!file) {
        std::cerr << "Failed to open pipe from file descriptor: " << fd << std::endl;
        return false;
    }
    
    try {
        // Read from file descriptor line-by-line
        char buffer[4096];
        std::string content;
        
        while (fgets(buffer, sizeof(buffer), file) != nullptr) {
            content += buffer;
        }
        
        fclose(file);
        
        // Load from the accumulated content
        std::istringstream iss(content);
        return loadFromStream(iss);
    } catch (const std::exception& e) {
        std::cerr << "Error loading from pipe: " << e.what() << std::endl;
        fclose(file);
        return false;
    }
    #endif
}

bool OopParser::saveToPipe(int fd) const {
    #ifdef _WIN32
    std::cerr << "Pipe operations not supported on Windows" << std::endl;
    return false;
    #else
    // Use fdopen to convert file descriptor to stream
    FILE* file = fdopen(fd, "w");
    if (!file) {
        std::cerr << "Failed to open pipe to file descriptor: " << fd << std::endl;
        return false;
    }
    
    try {
        // Save to string first
        std::ostringstream oss;
        if (!saveToStream(oss)) {
            fclose(file);
            return false;
        }
        
        std::string content = oss.str();
        
        // Write to file descriptor
        size_t written = fwrite(content.c_str(), 1, content.length(), file);
        fflush(file);
        
        bool success = (written == content.length());
        fclose(file);
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "Error saving to pipe: " << e.what() << std::endl;
        fclose(file);
        return false;
    }
    #endif
}

} // namespace ioc_config
