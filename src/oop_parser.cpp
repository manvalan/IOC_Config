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

// Include nlohmann/json for JSON support
#include <nlohmann/json.hpp>

// Include yaml-cpp for YAML support (if available)
#ifdef IOC_CONFIG_YAML_SUPPORT
#include <yaml-cpp/yaml.h>
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

OopParser ConfigBuilder::build() const {
    OopParser parser;
    for (const auto& section : sections_) {
        // We need to set parameters through the parser
        for (const auto& [key, param] : section.parameters) {
            parser.setParameter(section.name, key, param.value);
        }
    }
    return parser;
}

OopParser ConfigBuilder::getParser() const {
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

} // namespace ioc_config
