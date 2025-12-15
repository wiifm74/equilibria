// Basic tests for IPC protocol serialization and parsing
#include "../include/ipc_protocol.hpp"
#include "../include/json_utils.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace equilibria;

void test_telemetry_serialization() {
    std::cout << "Testing telemetry serialization..." << std::endl;
    
    ipc::TelemetryPayload telemetry;
    telemetry.timestamp_ms = 1234567890;
    telemetry.mode = ipc::Mode::IDLE;
    telemetry.temps.vapour_head = 78.2;
    telemetry.temps.boiler_liquid = 91.5;
    telemetry.temps.pcb_environment = 42.1;
    telemetry.pressures.ambient = 101.3;
    telemetry.pressures.vapour = std::nullopt;
    telemetry.flow_ml_min = 240.0;
    telemetry.valves.reflux_control = 65;
    telemetry.valves.product_takeoff = 30;
    telemetry.heaters.heater_1 = 70;
    telemetry.heaters.heater_2 = 70;
    telemetry.faults = {};
    
    std::string json = json::serialize_telemetry(telemetry);
    
    // Verify basic structure
    assert(json.find("\"timestamp_ms\":1234567890") != std::string::npos);
    assert(json.find("\"mode\":\"IDLE\"") != std::string::npos);
    assert(json.find("\"vapour_head\":78.2") != std::string::npos);
    assert(json.find("\"vapour\":null") != std::string::npos);
    assert(json.find("\"reflux_control\":65") != std::string::npos);
    
    std::cout << "  ✓ Telemetry serialization passed" << std::endl;
}

void test_ack_serialization() {
    std::cout << "Testing ACK serialization..." << std::endl;
    
    ipc::AckPayload ack;
    ack.command = "set_mode";
    ack.status = ipc::AckStatus::OK;
    ack.message = "Mode set successfully";
    
    std::string json = json::serialize_ack(ack);
    
    assert(json.find("\"command\":\"set_mode\"") != std::string::npos);
    assert(json.find("\"status\":\"ok\"") != std::string::npos);
    assert(json.find("\"message\":\"Mode set successfully\"") != std::string::npos);
    
    std::cout << "  ✓ ACK serialization passed" << std::endl;
}

void test_message_serialization() {
    std::cout << "Testing message serialization..." << std::endl;
    
    std::string payload = "{\"mode\":\"IDLE\"}";
    std::string message = json::serialize_message(ipc::MessageType::SET_MODE, payload);
    
    assert(message.find("\"version\":\"v0\"") != std::string::npos);
    assert(message.find("\"type\":\"set_mode\"") != std::string::npos);
    assert(message.find("\"payload\":{\"mode\":\"IDLE\"}") != std::string::npos);
    assert(message.back() == '\n'); // Newline delimiter
    
    std::cout << "  ✓ Message serialization passed" << std::endl;
}

void test_message_parsing() {
    std::cout << "Testing message parsing..." << std::endl;
    
    std::string json = "{\"version\":\"v0\",\"type\":\"get_telemetry\",\"payload\":{}}";
    auto msg = json::parse_message(json);
    
    assert(msg.has_value());
    assert(msg->version == "v0");
    assert(msg->type == "get_telemetry");
    assert(msg->payload_json == "{}");
    
    std::cout << "  ✓ Message parsing passed" << std::endl;
}

void test_set_mode_parsing() {
    std::cout << "Testing set_mode parsing..." << std::endl;
    
    std::string payload_json = "{\"mode\":\"ACTIVE\"}";
    auto payload = json::parse_set_mode(payload_json);
    
    assert(payload.has_value());
    assert(payload->mode == "ACTIVE");
    
    std::cout << "  ✓ set_mode parsing passed" << std::endl;
}

void test_set_targets_parsing() {
    std::cout << "Testing set_targets parsing..." << std::endl;
    
    std::string payload_json = "{\"target_abv\":95.0,\"target_flow\":300.0}";
    auto payload = json::parse_set_targets(payload_json);
    
    assert(payload.has_value());
    assert(payload->target_abv.has_value());
    assert(std::abs(payload->target_abv.value() - 95.0) < 0.01);
    assert(payload->target_flow.has_value());
    assert(std::abs(payload->target_flow.value() - 300.0) < 0.01);
    
    std::cout << "  ✓ set_targets parsing passed" << std::endl;
}

void test_json_escaping() {
    std::cout << "Testing JSON string escaping..." << std::endl;
    
    std::string input = "test\"quote\nand\\backslash";
    std::string escaped = json::escape_json_string(input);
    
    assert(escaped.find("\\\"") != std::string::npos);
    assert(escaped.find("\\n") != std::string::npos);
    assert(escaped.find("\\\\") != std::string::npos);
    
    std::cout << "  ✓ JSON escaping passed" << std::endl;
}

int main() {
    std::cout << "=== IPC Protocol Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        test_telemetry_serialization();
        test_ack_serialization();
        test_message_serialization();
        test_message_parsing();
        test_set_mode_parsing();
        test_set_targets_parsing();
        test_json_escaping();
        
        std::cout << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
