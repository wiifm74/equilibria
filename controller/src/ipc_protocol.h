#pragma once

#include <string>

// Forward declaration for minimal_json
namespace equilibria {
namespace minimal_json {
    class JsonObject;
}
}
namespace controller {

/**
 * @brief Controller state shared between IPC and control logic
 * 
 * POD-style structure representing the controller's operational state.
 */
struct ControllerState {
    enum class Mode {
        IDLE,
        ACTIVE
    };
    
    Mode mode;
    double target_abv;      // Target alcohol by volume (0-100%)
    double target_flow;     // Target flow rate (units TBD)
    
    ControllerState();
};

/**
 * @brief IPC protocol handler for controller commands and telemetry
 * 
 * Processes newline-delimited JSON messages from the API layer.
 * Message format: {"version": "v0", "type": "...", "payload": {...}}
 * 
 * Supported message types:
 * - get_telemetry: Request current state
 * - set_mode: Change controller mode (IDLE/ACTIVE)
 * - set_targets: Update target_abv and target_flow
 */
class IPCProtocol {
public:
    IPCProtocol();
    
    /**
     * @brief Process incoming IPC message
     * @param line Newline-delimited JSON message
     * @return JSON response (ack or error)
     */
    std::string process_message(const std::string& line);
    
    /**
     * @brief Generate telemetry message for current state
     * @return JSON telemetry message
     */
    std::string generate_telemetry() const;
    
    /**
     * @brief Get current controller state (read-only)
     */
    const ControllerState& get_state() const;

private:
    // Message handlers
    std::string handle_get_telemetry(const minimal_json::JsonObject& msg);
    std::string handle_set_mode(const minimal_json::JsonObject& msg);
    std::string handle_set_targets(const minimal_json::JsonObject& msg);
    
    // Response generators
    std::string create_ok_ack() const;
    std::string create_error_ack(const std::string& message) const;
    
    ControllerState state_;
};

} // namespace controller
} // namespace equilibria
