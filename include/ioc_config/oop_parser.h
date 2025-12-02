/**
 * @file oop_parser.h
 * @brief IOC_Config - OOP File Parser Library
 * 
 * A modern C++ library for parsing, manipulating, and converting 
 * OOP (OrbFit-style) configuration files used by IOoccultCalc.
 * 
 * @author Michele Bigi (mikbigi@gmail.com)
 * @version 1.2.0
 * @date 2025-12-02
 * @license MIT
 */

#ifndef IOC_CONFIG_OOP_PARSER_H
#define IOC_CONFIG_OOP_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace ioc_config {

/**
 * @brief Structure to represent a configuration parameter
 */
struct ConfigParameter {
    std::string key;           ///< Parameter key (e.g., ".id", ".name")
    std::string value;         ///< Parameter value as string
    std::string type;          ///< Data type (string, float, int, bool)

    /**
     * @brief Convert parameter value to string
     * @return String representation of the value
     */
    std::string asString() const;

    /**
     * @brief Convert parameter value to double
     * @return Double value (throws if conversion fails)
     */
    double asDouble() const;

    /**
     * @brief Convert parameter value to integer
     * @return Integer value (throws if conversion fails)
     */
    int asInt() const;

    /**
     * @brief Convert parameter value to boolean
     * @return Boolean value (throws if conversion fails)
     */
    bool asBoolean() const;

    /**
     * @brief Convert parameter value to vector of strings
     * @return Vector of strings (throws if conversion fails)
     */
    std::vector<std::string> asStringVector() const;
};

/**
 * @brief Constraint/Range definition for parameters
 * Supports various range formats:
 *  - "1..100" - from 1 to 100 (inclusive)
 *  - "d >= 4" - greater than or equal to 4
 *  - "5 < d < 30" - between 5 and 30 (exclusive)
 *  - "d < 1000" - less than 1000
 *  - "1..N" - from 1 to catalog size (for special case)
 */
struct RangeConstraint {
    bool enabled;                   ///< Whether constraint is active
    std::string constraint_expr;    ///< Original constraint expression
    double min_value;               ///< Minimum value (or -inf if unbounded)
    double max_value;               ///< Maximum value (or +inf if unbounded)
    bool min_inclusive;             ///< Is minimum inclusive (true) or exclusive (false)
    bool max_inclusive;             ///< Is maximum inclusive (true) or exclusive (false)
    bool is_range_to_catalog;       ///< Special case: "1..N" means 1 to catalog size

    /**
     * @brief Default constructor
     */
    RangeConstraint() : enabled(false), min_value(-1e308), max_value(1e308), 
                        min_inclusive(true), max_inclusive(true), 
                        is_range_to_catalog(false) {}

    /**
     * @brief Parse constraint expression
     * @param expr Expression like "1..100", "d >= 4", "5 < d < 30"
     * @return True if parsing successful
     */
    bool parseExpression(const std::string& expr);

    /**
     * @brief Check if value satisfies constraint
     * @param value Value to check
     * @return True if value is within constraints
     */
    bool isSatisfied(double value) const;

    /**
     * @brief Get constraint as human-readable string
     * @return String description of constraint
     */
    std::string toString() const;
};

/**
 * @brief Parameter specification with constraints
 */
struct ParameterSpec {
    std::string key;                    ///< Parameter key
    bool required;                      ///< Is parameter required?
    std::string description;            ///< Parameter description
    std::string default_value;          ///< Default value if optional
    RangeConstraint constraint;         ///< Range/value constraints
    std::vector<std::string> allowed_values;  ///< List of allowed values (for enum types)

    /**
     * @brief Default constructor
     */
    ParameterSpec() : key(""), required(false), description("") {}

    /**
     * @brief Check if value is valid according to spec
     * @param value Value to validate
     * @return True if valid
     */
    bool isValid(const std::string& value) const;
};

/**
 * @brief Section specification with parameter requirements
 */
struct SectionSpec {
    std::string name;                           ///< Section name
    bool required;                              ///< Is entire section required?
    std::string description;                    ///< Section description
    std::map<std::string, ParameterSpec> params;  ///< Parameter specifications

    /**
     * @brief Default constructor
     */
    SectionSpec() : name(""), required(false), description("") {}

    /**
     * @brief Add parameter specification
     * @param spec Parameter specification to add
     */
    void addParameter(const ParameterSpec& spec) {
        params[spec.key] = spec;
    }

    /**
     * @brief Get parameter specification
     * @param key Parameter key
     * @return Pointer to spec or nullptr if not found
     */
    ParameterSpec* getParameterSpec(const std::string& key) {
        auto it = params.find(key);
        return it != params.end() ? &it->second : nullptr;
    }
};

/**
 * @brief Configuration schema for validation
 */
struct ConfigSchema {
    std::string name;                           ///< Schema name
    std::string version;                        ///< Schema version
    std::map<std::string, SectionSpec> sections;  ///< Section specifications

    /**
     * @brief Add section specification
     * @param spec Section specification
     */
    void addSection(const SectionSpec& spec) {
        sections[spec.name] = spec;
    }

    /**
     * @brief Get section specification
     * @param name Section name
     * @return Pointer to spec or nullptr
     */
    SectionSpec* getSectionSpec(const std::string& name) {
        auto it = sections.find(name);
        return it != sections.end() ? &it->second : nullptr;
    }

    /**
     * @brief Export schema to JSON Schema format
     * @return JSON Schema as nlohmann::json object
     */
    nlohmann::json toJsonSchema() const;

    /**
     * @brief Export schema to JSON Schema file
     * @param filepath Path to output JSON Schema file
     * @return True if successful
     */
    bool saveJsonSchema(const std::string& filepath) const;

    /**
     * @brief Export schema to formatted JSON Schema string
     * @param indent Indentation level
     * @return JSON Schema as string
     */
    std::string toJsonSchemaString(int indent = 2) const;
};

/**
 * @brief Enumeration for section types
 */
enum class SectionType {
    UNKNOWN = 0,
    OBJECT = 1,
    PROPAGATION = 2,
    ASTEROIDS = 3,
    TIME = 4,
    SEARCH = 5,
    DATABASE = 6,
    GAIA = 7,
    OBSERVER = 8,
    OUTPUT = 9,
    PERFORMANCE = 10,
    OCCULTATION = 11,
    FILTERS = 12
};

/**
 * @brief Structure to represent a configuration section
 */
struct ConfigSectionData {
    SectionType type;                                    ///< Section type enumeration
    std::string name;                                    ///< Section name (e.g., "object", "propag")
    std::map<std::string, ConfigParameter> parameters;   ///< Map of parameters in this section

    /**
     * @brief Convert section type to string
     * @param type Section type
     * @return String representation
     */
    static std::string sectionTypeToString(SectionType type);

    /**
     * @brief Convert string to section type
     * @param name Section name
     * @return Section type enumeration
     */
    static SectionType stringToSectionType(const std::string& name);

    /**
     * @brief Get parameter by key
     * @param key Parameter key
     * @return Pointer to parameter or nullptr if not found
     */
    ConfigParameter* getParameter(const std::string& key);
    const ConfigParameter* getParameter(const std::string& key) const;
};

/**
 * @brief Main OOP File Parser class
 * 
 * This class handles parsing and manipulation of OOP (OrbFit-style) 
 * configuration files used by IOccultCalc.
 */
class OopParser {
public:
    /**
     * @brief Default constructor
     */
    OopParser();

    /**
     * @brief Constructor with file path
     * @param filepath Path to the OOP configuration file
     */
    explicit OopParser(const std::string& filepath);

    /**
     * @brief Destructor
     */
    ~OopParser() = default;

    /**
     * @brief Move constructor
     */
    OopParser(OopParser&&) = default;

    /**
     * @brief Move assignment operator
     */
    OopParser& operator=(OopParser&&) = default;

    // Disable copy semantics
    OopParser(const OopParser&) = delete;
    OopParser& operator=(const OopParser&) = delete;

    /**
     * @brief Load configuration from OOP file
     * @param filepath Path to the OOP configuration file
     * @return True if successful, false otherwise
     */
    bool loadFromOop(const std::string& filepath);

    /**
     * @brief Save configuration to OOP file
     * @param filepath Path where to save the OOP configuration file
     * @return True if successful, false otherwise
     */
    bool saveToOop(const std::string& filepath) const;

    /**
     * @brief Load configuration from JSON file
     * @param filepath Path to the JSON configuration file
     * @return True if successful, false otherwise
     */
    bool loadFromJson(const std::string& filepath);

    /**
     * @brief Save configuration to JSON file
     * @param filepath Path where to save the JSON configuration file
     * @return True if successful, false otherwise
     */
    bool saveToJson(const std::string& filepath) const;

    /**
     * @brief Get all sections
     * @return Vector of ConfigSectionData structures
     */
    std::vector<ConfigSectionData> getAllSections() const;

    /**
     * @brief Get section by type
     * @param type Section type
     * @return Pointer to section or nullptr if not found
     */
    ConfigSectionData* getSection(SectionType type);
    const ConfigSectionData* getSection(SectionType type) const;

    /**
     * @brief Get section by name
     * @param name Section name (e.g., "object", "propag")
     * @return Pointer to section or nullptr if not found
     */
    ConfigSectionData* getSection(const std::string& name);
    const ConfigSectionData* getSection(const std::string& name) const;

    /**
     * @brief Add or update a parameter in a section
     * @param sectionName Name of the section
     * @param paramKey Parameter key (with or without leading dot)
     * @param value Parameter value
     * @return True if successful, false otherwise
     */
    bool setParameter(const std::string& sectionName, 
                      const std::string& paramKey, 
                      const std::string& value);

    /**
     * @brief Find parameter by key across all sections
     * @param paramKey Parameter key to search for
     * @return Pointer to first matching parameter or nullptr
     */
    ConfigParameter* findParameter(const std::string& paramKey);
    const ConfigParameter* findParameter(const std::string& paramKey) const;

    /**
     * @brief Validate configuration
     * @param errors Output vector to collect validation errors
     * @return True if valid, false otherwise
     */
    bool validate(std::vector<std::string>& errors) const;

    /**
     * @brief Validate configuration against schema with constraints
     * @param schema Schema to validate against
     * @param errors Output vector to collect validation errors
     * @return True if valid according to schema
     */
    bool validateWithSchema(const ConfigSchema& schema, 
                           std::vector<std::string>& errors) const;

    /**
     * @brief Set schema for validation
     * @param schema Schema to use for validation
     */
    void setSchema(const ConfigSchema& schema);

    /**
     * @brief Get current schema
     * @return Pointer to current schema or nullptr
     */
    const ConfigSchema* getSchema() const;

    /**
     * @brief Create default schema with standard sections
     * @return Newly created default schema
     */
    static ConfigSchema createDefaultSchema();

    /**
     * @brief Clear all sections
     */
    void clear();

    /**
     * @brief Get number of sections
     * @return Number of sections
     */
    size_t getSectionCount() const;

    /**
     * @brief Get last error message
     * @return Error message string
     */
    std::string getLastError() const;

    /**
     * @brief Load configuration from JSON string
     * @param jsonString JSON string containing configuration
     * @return True if successful, false otherwise
     */
    bool loadFromJsonString(const std::string& jsonString);

    /**
     * @brief Save configuration to JSON string
     * @return JSON string representation of configuration
     */
    std::string saveToJsonString() const;

    /**
     * @brief Load configuration from JSON object
     * @param jsonObj nlohmann::json object containing configuration
     * @return True if successful, false otherwise
     */
    bool loadFromJsonObject(const nlohmann::json& jsonObj);

    /**
     * @brief Save configuration as JSON object
     * @return nlohmann::json object representation
     */
    nlohmann::json saveToJsonObject() const;

    /**
     * @brief Get configuration as JSON (alias for saveToJsonObject)
     * @return JSON object with configuration
     */
    nlohmann::json toJson() const;

    /**
     * @brief Populate configuration from JSON (alias for loadFromJsonObject)
     * @param jsonObj JSON object with configuration
     * @return True if successful
     */
    bool fromJson(const nlohmann::json& jsonObj);

    /**
     * @brief Validate configuration against JSON schema
     * @param schemaJson Optional JSON schema for validation
     * @param errors Output vector to collect validation errors
     * @return True if valid against schema
     */
    bool validateAgainstSchema(const nlohmann::json& schemaJson, 
                               std::vector<std::string>& errors) const;

    /**
     * @brief Get configuration as formatted JSON string (pretty-printed)
     * @param indent Number of spaces for indentation (default: 2)
     * @return Pretty-printed JSON string
     */
    std::string toJsonString(int indent = 2) const;

    // ============ YAML Support Methods ============
    
    /**
     * @brief Load configuration from YAML file
     * @param filepath Path to YAML file
     * @return True if successful
     */
    bool loadFromYaml(const std::string& filepath);

    /**
     * @brief Save configuration to YAML file
     * @param filepath Path to YAML file to create/overwrite
     * @return True if successful
     */
    bool saveToYaml(const std::string& filepath) const;

    /**
     * @brief Load configuration from YAML string
     * @param yamlString YAML formatted string
     * @return True if successful
     */
    bool loadFromYamlString(const std::string& yamlString);

    /**
     * @brief Get configuration as YAML string
     * @return YAML formatted string
     */
    std::string saveToYamlString() const;

    /**
     * @brief Trim whitespace from string (static utility)
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Split string by delimiter (static utility)
     * @param str String to split
     * @param delimiter Character delimiter
     * @return Vector of split strings
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Detect data type of a value (static utility)
     * @param value Value string
     * @return Type string ("string", "float", "int", "bool", "array")
     */
    static std::string detectType(const std::string& value);

private:
    std::vector<ConfigSectionData> sections_;           ///< Configuration sections
    mutable std::string lastError_;                     ///< Last error message
    std::unique_ptr<ConfigSchema> schema_;              ///< Current validation schema

    /**
     * @brief Parse a single line from OOP file
     * @param line Line to parse
     * @param currentSection Current section being processed
     * @return True if line was successfully parsed
     */
    bool parseLine(const std::string& line, ConfigSectionData& currentSection);

    /**
     * @brief Check if line is a comment
     * @param line Line to check
     * @return True if line is a comment
     */
    static bool isComment(const std::string& line);

    /**
     * @brief Check if line starts a new section
     * @param line Line to check
     * @return Section name if it's a section header, empty string otherwise
     */
    static std::string isSection(const std::string& line);

    /**
     * @brief Parse array value enclosed in brackets
     * @param value String value containing array
     * @return Vector of parsed array elements
     */
    static std::vector<std::string> parseArrayValue(const std::string& value);

#ifdef IOC_CONFIG_YAML_SUPPORT
    /**
     * @brief Parse a YAML node into configuration sections
     * @param config YAML node to parse
     * @return True if parsing was successful
     */
    bool loadFromYamlNode(const YAML::Node& config);
#endif
};

/**
 * @brief Fluent builder for constructing OOP configurations
 * 
 * Allows building configurations programmatically with a fluent API:
 * @code
 * ConfigBuilder builder;
 * builder.addSection("object")
 *        .addParameter("id", "17030")
 *        .addParameter("name", "Asteroid 17030")
 *        .endSection()
 *        .addSection("search")
 *        .addParameter("max_magnitude", "16.5")
 *        .endSection()
 *        .build();  // Returns OopParser
 * @endcode
 */
class ConfigBuilder {
public:
    /**
     * @brief Default constructor
     */
    ConfigBuilder();

    /**
     * @brief Add a new section
     * @param sectionName Name of the section to add
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& addSection(const std::string& sectionName);

    /**
     * @brief Add a parameter to the current section
     * @param key Parameter key
     * @param value Parameter value
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& addParameter(const std::string& key, const std::string& value);

    /**
     * @brief End the current section (optional, for clarity)
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& endSection();

    /**
     * @brief Add multiple parameters at once
     * @param params Map of key-value pairs
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& addParameters(const std::map<std::string, std::string>& params);

    /**
     * @brief Build and return the OopParser
     * @return Constructed OopParser with all sections and parameters
     */
    OopParser build() const;

    /**
     * @brief Get the parser without resetting builder state
     * @return Copy of current parser state
     */
    OopParser getParser() const;

    /**
     * @brief Clear all sections (reset builder)
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& clear();

    /**
     * @brief Add section from another parser
     * @param parser Source parser
     * @param sectionName Name of section to copy
     * @return Reference to this builder for method chaining
     */
    ConfigBuilder& addSectionFrom(const OopParser& parser, const std::string& sectionName);

    /**
     * @brief Get number of sections in builder
     * @return Section count
     */
    size_t getSectionCount() const;

    /**
     * @brief Get all section names
     * @return Vector of section names
     */
    std::vector<std::string> getSectionNames() const;

private:
    std::vector<ConfigSectionData> sections_;      ///< Built sections
    std::string currentSection_;                   ///< Name of current section
    size_t currentSectionIndex_;                   ///< Index of current section
};

/**
 * @brief Convert OOP file to JSON
 * @param oopFilepath Path to OOP file
 * @param jsonFilepath Path to output JSON file
 * @return True if successful, false otherwise
 */
bool convertOopToJson(const std::string& oopFilepath, 
                      const std::string& jsonFilepath);

/**
 * @brief Convert JSON file to OOP
 * @param jsonFilepath Path to JSON file
 * @param oopFilepath Path to output OOP file
 * @return True if successful, false otherwise
 */
bool convertJsonToOop(const std::string& jsonFilepath, 
                      const std::string& oopFilepath);

/**
 * @brief Get library version
 * @return Version string
 */
std::string getLibraryVersion();

} // namespace ioc_config

#endif // IOC_CONFIG_OOP_PARSER_H
