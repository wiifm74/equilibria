#include "test_framework.hpp"
#include "../src/ipc_protocol.h"
#include <nlohmann/json.hpp>

using namespace equilibria::test;
using namespace equilibria::controller;
using json = nlohmann::json;

// Helper to check if response is error ack
bool is_error_ack(const std::string& response) {
    try {
        json j = json::parse(response);
        return j["type"] == "ack" && j["payload"]["status"] == "error";
    } catch (...) {
        return false;
    }
}

// Helper to check if response is ok ack
bool is_ok_ack(const std::string& response) {
    try {
        json j = json::parse(response);
        return j["type"] == "ack" && j["payload"]["status"] == "ok";
    } catch (...) {
        return false;
    }
}

// ========== Valid JSON Tests ==========

TEST(valid_json_get_telemetry) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"get_telemetry","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
}

TEST(valid_json_set_mode_idle) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_mode","payload":{"mode":"IDLE"}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
    ASSERT_EQ(ControllerState::Mode::IDLE, protocol.get_state().mode);
}

TEST(valid_json_set_mode_active) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_mode","payload":{"mode":"ACTIVE"}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
    ASSERT_EQ(ControllerState::Mode::ACTIVE, protocol.get_state().mode);
}

TEST(valid_json_set_targets) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":42.5,"target_flow":1.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
    ASSERT_EQ(42.5, protocol.get_state().target_abv);
    ASSERT_EQ(1.5, protocol.get_state().target_flow);
}

// ========== Invalid JSON Tests ==========

TEST(invalid_json_malformed) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":})"; // Malformed JSON
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "parse error");
}

TEST(invalid_json_empty_string) {
    IPCProtocol protocol;
    std::string msg = "";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
}

TEST(invalid_json_not_object) {
    IPCProtocol protocol;
    std::string msg = R"(["array","not","object"])";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
}

// ========== Version Tests ==========

TEST(missing_version_field) {
    IPCProtocol protocol;
    std::string msg = R"({"type":"get_telemetry","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "version");
}

TEST(unknown_version) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v1","type":"get_telemetry","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "Unknown version");
}

TEST(invalid_version_type) {
    IPCProtocol protocol;
    std::string msg = R"({"version":123,"type":"get_telemetry","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
}

// ========== Missing Field Tests ==========

TEST(missing_type_field) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "type");
}

TEST(missing_payload_field) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"get_telemetry"})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "payload");
}

TEST(unknown_message_type) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"unknown_command","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "Unknown message type");
}

// ========== set_mode Payload Validation ==========

TEST(set_mode_missing_mode_field) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_mode","payload":{}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "mode");
}

TEST(set_mode_invalid_mode_type) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_mode","payload":{"mode":123}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "must be a string");
}

TEST(set_mode_invalid_mode_value) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_mode","payload":{"mode":"UNKNOWN"}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "Invalid mode");
}

// ========== set_targets Payload Validation ==========

TEST(set_targets_missing_target_abv) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_flow":1.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "target_abv");
}

TEST(set_targets_missing_target_flow) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":42.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "target_flow");
}

TEST(set_targets_invalid_abv_type) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":"not_a_number","target_flow":1.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "must be a number");
}

TEST(set_targets_invalid_flow_type) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":42.5,"target_flow":"not_a_number"}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "must be a number");
}

TEST(set_targets_abv_out_of_range_high) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":150.0,"target_flow":1.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "out of range");
}

TEST(set_targets_abv_out_of_range_low) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":-10.0,"target_flow":1.5}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "out of range");
}

TEST(set_targets_negative_flow) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":42.5,"target_flow":-1.0}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_error_ack(response));
    ASSERT_CONTAINS(response, "cannot be negative");
}

TEST(set_targets_boundary_values) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":0.0,"target_flow":0.0}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
    ASSERT_EQ(0.0, protocol.get_state().target_abv);
    ASSERT_EQ(0.0, protocol.get_state().target_flow);
}

TEST(set_targets_max_abv) {
    IPCProtocol protocol;
    std::string msg = R"({"version":"v0","type":"set_targets","payload":{"target_abv":100.0,"target_flow":5.0}})";
    std::string response = protocol.process_message(msg);
    
    ASSERT_TRUE(is_ok_ack(response));
    ASSERT_EQ(100.0, protocol.get_state().target_abv);
}

// ========== State Persistence Tests ==========

TEST(multiple_commands_state_persists) {
    IPCProtocol protocol;
    
    // Set mode to ACTIVE
    std::string msg1 = R"({"version":"v0","type":"set_mode","payload":{"mode":"ACTIVE"}})";
    protocol.process_message(msg1);
    
    // Set targets
    std::string msg2 = R"({"version":"v0","type":"set_targets","payload":{"target_abv":42.5,"target_flow":1.5}})";
    protocol.process_message(msg2);
    
    // Verify state persists
    ASSERT_EQ(ControllerState::Mode::ACTIVE, protocol.get_state().mode);
    ASSERT_EQ(42.5, protocol.get_state().target_abv);
    ASSERT_EQ(1.5, protocol.get_state().target_flow);
}

TEST(set_targets_updates_both_values) {
    IPCProtocol protocol;
    
    // Set initial targets
    std::string msg1 = R"({"version":"v0","type":"set_targets","payload":{"target_abv":40.0,"target_flow":2.0}})";
    protocol.process_message(msg1);
    
    // Update targets
    std::string msg2 = R"({"version":"v0","type":"set_targets","payload":{"target_abv":50.0,"target_flow":3.0}})";
    protocol.process_message(msg2);
    
    // Verify both values updated
    ASSERT_EQ(50.0, protocol.get_state().target_abv);
    ASSERT_EQ(3.0, protocol.get_state().target_flow);
}

// Main entry point
int main() {
    return equilibria::test::TestRunner::instance().run_all();
}
