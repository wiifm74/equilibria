#include "ipc_protocol.h"
#include "../third_party/json/minimal_json.hpp"
#include <sstream>
#include <string>
#include <iostream>

using json = equilibria::minimal_json::JsonObject;

namespace equilibria {
namespace controller {

// ControllerState implementation
ControllerState::ControllerState()
    : mode(Mode::IDLE)
    , target_abv(0.0)
    , target_flow(0.0)
{}

// IPCProtocol implementation
IPCProtocol::IPCProtocol()
    : state_()
{}

std::string IPCProtocol::process_message(const std::string& line) {
    try {
        json msg = json::parse(line);
        
        // Validate version
        if (!msg.has("version")) {
            return create_error_ack("Missing 'version' field");
        }
        
        std::string version = msg.getString("version");
        if (version != "v0") {
            return create_error_ack("Unknown version: " + version);
        }
        
        // Validate type
        if (!msg.has("type")) {
            return create_error_ack("Missing 'type' field");
        }
        
        std::string type = msg.getString("type");
        
        // Note: payload is encoded in the same message object for minimal_json
        // Route to handler
        if (type == "get_telemetry") {
            return handle_get_telemetry(msg);
        } else if (type == "set_mode") {
            return handle_set_mode(msg);
        } else if (type == "set_targets") {
            return handle_set_targets(msg);
        } else {
            return create_error_ack("Unknown message type: " + type);
        }
        
    } catch (const std::exception& e) {
        return create_error_ack(std::string("Error: ") + e.what());
    }
}

std::string IPCProtocol::handle_get_telemetry(const json& msg) {
    // get_telemetry doesn't require additional validation
    // Return OK ack - actual telemetry will be sent separately
    return create_ok_ack();
}

std::string IPCProtocol::handle_set_mode(const json& msg) {
    if (!msg.has("mode")) {
        return create_error_ack("Missing 'mode' field");
    }
    
    std::string mode_str = msg.getString("mode");
    
    if (mode_str == "IDLE") {
        state_.mode = ControllerState::Mode::IDLE;
        std::cout << "[IPC] Mode changed to IDLE\n";
    } else if (mode_str == "ACTIVE") {
        state_.mode = ControllerState::Mode::ACTIVE;
        std::cout << "[IPC] Mode changed to ACTIVE\n";
    } else {
        return create_error_ack("Invalid mode: " + mode_str);
    }
    
    return create_ok_ack();
}

std::string IPCProtocol::handle_set_targets(const json& payload) {
    // Validate target_abv
    if (!payload.contains("target_abv")) {
        return create_error_ack("Missing 'target_abv' in payload");
    }msg) {
    // Validate target_abv
    if (!msg.has("target_abv")) {
        return create_error_ack("Missing 'target_abv' field");
    }
    
    // Validate target_flow
    if (!msg.has("target_flow")) {
        return create_error_ack("Missing 'target_flow' field");
    }
    
    double abv = msg.getNumber("target_abv");
    double flow = msg.getNumber("target_flow")abv out of range (0-100)");
    }
    if (flow < 0.0) {
        return create_error_ack("target_flow cannot be negative");
    }
    
    state_.target_abv = abv;
    state_.target_flow = flow;
    
    std::cout << "[IPC] Targets updated: ABV=" << abv << "%, Flow=" << flow << "\n";
    
    return create_ok_ack();
}

std::string IPCProtocol::generate_telemetry() const {
    json msg;
    msg["version"] = "v0";
    msg["type"] = "telemetry";
    
    json payload;
    payload["mode"] = (state_.mode == ControllerState::Mode::IDLE) ? "IDLE" : "ACTIVE";
    payload["target_abv"] = state_.target_abv;
    pay.set("version", "v0");
    msg.set("type", "telemetry");
    msg.set("mode", (state_.mode == ControllerState::Mode::IDLE) ? "IDLE" : "ACTIVE");
    msg.set("target_abv", state_.target_abv);
    msg.set("target_flow", state_.target_flow)
    ack["version"] = "v0";
    ack["type"] = "ack";
    
    json payload;
    payload["status"] = "ok";
    
    ack["payload"] = payload;
    
    return ack.dump();
}
.set("version", "v0");
    ack.set("type", "ack");
    ack.set("status", "ok")
    payload["status"] = "error";
    payload["message"] = message;
    
    ack["payload"] = payload;
    .set("version", "v0");
    ack.set("type", "ack");
    ack.set("status", "error");
    ack.set("message", message)
}

} // namespace controller
} // namespace equilibria