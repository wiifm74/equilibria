#ifndef EQUILIBRIA_IPC_PROTOCOL_HPP
#define EQUILIBRIA_IPC_PROTOCOL_HPP

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <chrono>

namespace equilibria {
namespace ipc {

// Protocol version
constexpr const char* PROTOCOL_VERSION = "v0";

// Default TCP settings
constexpr const char* DEFAULT_HOST = "127.0.0.1";
constexpr int DEFAULT_PORT = 7002;

// Message types
namespace MessageType {
    constexpr const char* GET_TELEMETRY = "get_telemetry";
    constexpr const char* SET_MODE = "set_mode";
    constexpr const char* SET_TARGETS = "set_targets";
    constexpr const char* TELEMETRY = "telemetry";
    constexpr const char* ACK = "ack";
}

// Operating modes
namespace Mode {
    constexpr const char* IDLE = "IDLE";
    constexpr const char* ACTIVE = "ACTIVE";
}

// Status codes for ACK
namespace AckStatus {
    constexpr const char* OK = "ok";
    constexpr const char* ERROR = "error";
}

// Temperature readings (Celsius)
struct TemperatureReadings {
    std::optional<double> vapour_head;
    std::optional<double> boiler_liquid;
    std::optional<double> pcb_environment;
};

// Pressure readings (kPa)
struct PressureReadings {
    std::optional<double> ambient;
    std::optional<double> vapour;
};

// Valve positions (0-100%)
struct ValvePositions {
    int reflux_control;
    int product_takeoff;
};

// Heater power levels (0-100%)
struct HeaterLevels {
    int heater_1;
    int heater_2;
};

// Telemetry payload
struct TelemetryPayload {
    int64_t timestamp_ms;
    std::string mode;
    TemperatureReadings temps;
    PressureReadings pressures;
    std::optional<double> flow_ml_min;
    ValvePositions valves;
    HeaterLevels heaters;
    std::vector<std::string> faults;
};

// Set mode payload
struct SetModePayload {
    std::string mode;
};

// Set targets payload
struct SetTargetsPayload {
    std::optional<double> target_abv;
    std::optional<double> target_flow;
};

// ACK payload
struct AckPayload {
    std::string command;
    std::string status;
    std::optional<std::string> message;
};

// Base message structure
struct Message {
    std::string version;
    std::string type;
    std::string payload_json; // Raw JSON payload as string
};

// Helper to get current timestamp in milliseconds
inline int64_t get_timestamp_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

} // namespace ipc
} // namespace equilibria

#endif // EQUILIBRIA_IPC_PROTOCOL_HPP
