#ifndef EQUILIBRIA_JSON_UTILS_HPP
#define EQUILIBRIA_JSON_UTILS_HPP

#include "ipc_protocol.hpp"
#include <string>
#include <sstream>
#include <optional>

namespace equilibria {
namespace json {

// Simple JSON string escaping
inline std::string escape_json_string(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    
    for (char c : input) {
        switch (c) {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c < 0x20) {
                    // Control character - use unicode escape
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    output += buf;
                } else {
                    output += c;
                }
        }
    }
    
    return output;
}

// Serialize optional double to JSON
inline std::string optional_double_to_json(const std::optional<double>& opt) {
    if (opt.has_value()) {
        return std::to_string(opt.value());
    }
    return "null";
}

// Serialize telemetry payload to JSON
inline std::string serialize_telemetry(const ipc::TelemetryPayload& telemetry) {
    std::ostringstream oss;
    
    oss << "{";
    oss << "\"timestamp_ms\":" << telemetry.timestamp_ms << ",";
    oss << "\"mode\":\"" << escape_json_string(telemetry.mode) << "\",";
    
    // Temps
    oss << "\"temps\":{";
    oss << "\"vapour_head\":" << optional_double_to_json(telemetry.temps.vapour_head) << ",";
    oss << "\"boiler_liquid\":" << optional_double_to_json(telemetry.temps.boiler_liquid) << ",";
    oss << "\"pcb_environment\":" << optional_double_to_json(telemetry.temps.pcb_environment);
    oss << "},";
    
    // Pressures
    oss << "\"pressures\":{";
    oss << "\"ambient\":" << optional_double_to_json(telemetry.pressures.ambient) << ",";
    oss << "\"vapour\":" << optional_double_to_json(telemetry.pressures.vapour);
    oss << "},";
    
    // Flow
    oss << "\"flow_ml_min\":" << optional_double_to_json(telemetry.flow_ml_min) << ",";
    
    // Valves
    oss << "\"valves\":{";
    oss << "\"reflux_control\":" << telemetry.valves.reflux_control << ",";
    oss << "\"product_takeoff\":" << telemetry.valves.product_takeoff;
    oss << "},";
    
    // Heaters
    oss << "\"heaters\":{";
    oss << "\"heater_1\":" << telemetry.heaters.heater_1 << ",";
    oss << "\"heater_2\":" << telemetry.heaters.heater_2;
    oss << "},";
    
    // Faults
    oss << "\"faults\":[";
    for (size_t i = 0; i < telemetry.faults.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << escape_json_string(telemetry.faults[i]) << "\"";
    }
    oss << "]";
    
    oss << "}";
    
    return oss.str();
}

// Serialize ACK payload to JSON
inline std::string serialize_ack(const ipc::AckPayload& ack) {
    std::ostringstream oss;
    
    oss << "{";
    oss << "\"command\":\"" << escape_json_string(ack.command) << "\",";
    oss << "\"status\":\"" << escape_json_string(ack.status) << "\"";
    
    if (ack.message.has_value()) {
        oss << ",\"message\":\"" << escape_json_string(ack.message.value()) << "\"";
    }
    
    oss << "}";
    
    return oss.str();
}

// Serialize a complete message
inline std::string serialize_message(const std::string& type, const std::string& payload_json) {
    std::ostringstream oss;
    
    oss << "{";
    oss << "\"version\":\"" << ipc::PROTOCOL_VERSION << "\",";
    oss << "\"type\":\"" << escape_json_string(type) << "\",";
    oss << "\"payload\":" << payload_json;
    oss << "}\n";  // Newline delimiter
    
    return oss.str();
}

// Type-safe message creation helpers (recommended for v0)
inline std::string create_telemetry_message(const ipc::TelemetryPayload& telemetry) {
    return serialize_message(ipc::MessageType::TELEMETRY, serialize_telemetry(telemetry));
}

inline std::string create_ack_message(const ipc::AckPayload& ack) {
    return serialize_message(ipc::MessageType::ACK, serialize_ack(ack));
}

// Simple JSON value extraction (minimal parser for our needs)
// Finds a field in JSON and returns its value as string
inline std::optional<std::string> extract_json_field(const std::string& json, const std::string& field) {
    std::string search = "\"" + field + "\"";
    size_t pos = json.find(search);
    
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    
    // Find the colon after the field name
    size_t colon_pos = json.find(':', pos);
    if (colon_pos == std::string::npos) {
        return std::nullopt;
    }
    
    // Skip whitespace after colon
    size_t value_start = colon_pos + 1;
    while (value_start < json.size() && std::isspace(json[value_start])) {
        ++value_start;
    }
    
    if (value_start >= json.size()) {
        return std::nullopt;
    }
    
    // Determine value type and extract
    char first_char = json[value_start];
    
    if (first_char == '"') {
        // String value - find unescaped closing quote
        size_t end_quote = value_start + 1;
        while (end_quote < json.size()) {
            if (json[end_quote] == '"') {
                // Count preceding backslashes
                size_t backslash_count = 0;
                size_t check_pos = end_quote - 1;
                while (check_pos >= value_start && json[check_pos] == '\\') {
                    ++backslash_count;
                    if (check_pos == 0) break;
                    --check_pos;
                }
                // If even number of backslashes (including 0), the quote is unescaped
                if (backslash_count % 2 == 0) {
                    return json.substr(value_start + 1, end_quote - value_start - 1);
                }
            }
            ++end_quote;
        }
        return std::nullopt;
    } else if (first_char == '{') {
        // Object value - find matching brace
        int brace_count = 1;
        size_t end_pos = value_start + 1;
        while (end_pos < json.size() && brace_count > 0) {
            if (json[end_pos] == '{') ++brace_count;
            else if (json[end_pos] == '}') --brace_count;
            ++end_pos;
        }
        return json.substr(value_start, end_pos - value_start);
    } else {
        // Number, boolean, or null - find end delimiter
        size_t end_pos = value_start;
        while (end_pos < json.size() && 
               json[end_pos] != ',' && 
               json[end_pos] != '}' && 
               json[end_pos] != ']' &&
               !std::isspace(json[end_pos])) {
            ++end_pos;
        }
        return json.substr(value_start, end_pos - value_start);
    }
}

// Parse a complete message
inline std::optional<ipc::Message> parse_message(const std::string& json) {
    ipc::Message msg;
    
    auto version = extract_json_field(json, "version");
    if (!version.has_value()) return std::nullopt;
    msg.version = version.value();
    
    auto type = extract_json_field(json, "type");
    if (!type.has_value()) return std::nullopt;
    msg.type = type.value();
    
    auto payload = extract_json_field(json, "payload");
    if (!payload.has_value()) return std::nullopt;
    msg.payload_json = payload.value();
    
    return msg;
}

// Extract optional double from JSON field
inline std::optional<double> extract_optional_double(const std::string& json, const std::string& field) {
    auto value_str = extract_json_field(json, field);
    if (!value_str.has_value()) return std::nullopt;
    
    try {
        return std::stod(value_str.value());
    } catch (...) {
        return std::nullopt;
    }
}

// Parse set_mode payload
inline std::optional<ipc::SetModePayload> parse_set_mode(const std::string& payload_json) {
    auto mode = extract_json_field(payload_json, "mode");
    if (!mode.has_value()) return std::nullopt;
    
    ipc::SetModePayload payload;
    payload.mode = mode.value();
    return payload;
}

// Parse set_targets payload
inline std::optional<ipc::SetTargetsPayload> parse_set_targets(const std::string& payload_json) {
    ipc::SetTargetsPayload payload;
    
    payload.target_abv = extract_optional_double(payload_json, "target_abv");
    payload.target_flow = extract_optional_double(payload_json, "target_flow");
    
    return payload;
}

} // namespace json
} // namespace equilibria

#endif // EQUILIBRIA_JSON_UTILS_HPP
