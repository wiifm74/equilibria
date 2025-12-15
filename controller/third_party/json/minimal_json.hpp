#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace equilibria {
namespace minimal_json {

/**
 * @brief Minimal JSON parser for controller IPC
 * 
 * This is a fallback option if nlohmann/json is not available.
 * It supports only the minimal features needed for IPC:
 * - Parse JSON objects with string keys and string/number/bool values
 * - Create JSON objects and serialize to string
 * 
 * Limitations:
 * - No nested objects or arrays (add if needed)
 * - No escape sequence handling beyond basic quotes
 * - No unicode support
 * - Minimal error checking
 * 
 * For production use, prefer nlohmann/json.
 */
class JsonObject {
public:
    JsonObject() = default;

    // Parse from string
    static JsonObject parse(const std::string& jsonStr) {
        JsonObject obj;
        size_t pos = jsonStr.find('{');
        if (pos == std::string::npos) {
            throw std::runtime_error("Invalid JSON: missing opening brace");
        }

        pos++; // Skip opening brace
        while (pos < jsonStr.size()) {
            // Skip whitespace
            while (pos < jsonStr.size() && std::isspace(jsonStr[pos])) pos++;
            
            if (pos >= jsonStr.size() || jsonStr[pos] == '}') break;

            // Parse key
            if (jsonStr[pos] != '"') {
                throw std::runtime_error("Invalid JSON: expected key");
            }
            pos++; // Skip opening quote
            size_t keyStart = pos;
            while (pos < jsonStr.size() && jsonStr[pos] != '"') pos++;
            std::string key = jsonStr.substr(keyStart, pos - keyStart);
            pos++; // Skip closing quote

            // Skip whitespace and colon
            while (pos < jsonStr.size() && (std::isspace(jsonStr[pos]) || jsonStr[pos] == ':')) pos++;

            // Parse value
            if (jsonStr[pos] == '"') {
                // String value
                pos++; // Skip opening quote
                size_t valueStart = pos;
                while (pos < jsonStr.size() && jsonStr[pos] != '"') pos++;
                obj.strings_[key] = jsonStr.substr(valueStart, pos - valueStart);
                pos++; // Skip closing quote
            } else if (std::isdigit(jsonStr[pos]) || jsonStr[pos] == '-') {
                // Number value
                size_t valueStart = pos;
                while (pos < jsonStr.size() && (std::isdigit(jsonStr[pos]) || jsonStr[pos] == '.' || jsonStr[pos] == '-')) pos++;
                obj.strings_[key] = jsonStr.substr(valueStart, pos - valueStart);
            } else if (jsonStr.substr(pos, 4) == "true") {
                obj.strings_[key] = "true";
                pos += 4;
            } else if (jsonStr.substr(pos, 5) == "false") {
                obj.strings_[key] = "false";
                pos += 5;
            }

            // Skip whitespace and comma
            while (pos < jsonStr.size() && (std::isspace(jsonStr[pos]) || jsonStr[pos] == ',')) pos++;
        }

        return obj;
    }

    // Set values
    void set(const std::string& key, const std::string& value) {
        strings_[key] = value;
    }

    void set(const std::string& key, double value) {
        strings_[key] = std::to_string(value);
    }

    void set(const std::string& key, int value) {
        strings_[key] = std::to_string(value);
    }

    void set(const std::string& key, bool value) {
        strings_[key] = value ? "true" : "false";
    }

    // Get values
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = strings_.find(key);
        return (it != strings_.end()) ? it->second : defaultValue;
    }

    double getNumber(const std::string& key, double defaultValue = 0.0) const {
        auto it = strings_.find(key);
        if (it != strings_.end()) {
            return std::stod(it->second);
        }
        return defaultValue;
    }

    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = strings_.find(key);
        if (it != strings_.end()) {
            return it->second == "true";
        }
        return defaultValue;
    }

    bool has(const std::string& key) const {
        return strings_.find(key) != strings_.end();
    }

    // Serialize to JSON string
    std::string dump() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& pair : strings_) {
            if (!first) oss << ",";
            first = false;
            
            oss << "\"" << pair.first << "\":";
            
            // Determine if value needs quotes
            if (pair.second == "true" || pair.second == "false") {
                oss << pair.second;
            } else if (!pair.second.empty() && (std::isdigit(pair.second[0]) || pair.second[0] == '-')) {
                oss << pair.second;
            } else {
                oss << "\"" << pair.second << "\"";
            }
        }
        oss << "}";
        return oss.str();
    }

private:
    std::map<std::string, std::string> strings_;
};

} // namespace minimal_json
} // namespace equilibria
