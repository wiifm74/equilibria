#include "controller.h"
#include "telemetry_publisher.h"
#include "config.h"
#include <chrono>
#include <cstring>
#include <vector>

namespace equilibria {

// Telemetry message structure (POD, versioned)
struct TelemetryMessage {
    static constexpr uint8_t VERSION = 1;
    
    uint8_t version;
    uint64_t timestamp_ms;
    uint8_t mode;
    
    // Temperature readings (degC * 100, INT16_MAX = null)
    int16_t temp_vapour_head;
    int16_t temp_boiler_liquid;
    int16_t temp_pcb_environment;
    
    // Pressure readings (kPa * 100, INT16_MAX = null)
    int16_t pressure_ambient;
    int16_t pressure_vapour;
    
    // Flow rate (ml/min * 10)
    uint16_t flow_ml_min;
    
    // Valve states (0-100%)
    uint8_t valve_reflux_control;
    uint8_t valve_product_takeoff;
    
    // Heater states (0-100%)
    uint8_t heater_1;
    uint8_t heater_2;
    
    // Fault flags (bitfield)
    uint32_t faults;
    
    // Sensor presence map (bitfield)
    uint16_t sensor_presence;
} __attribute__((packed));

static_assert(sizeof(TelemetryMessage) <= 64, "Keep telemetry compact for IPC");

// Sensor presence bits
enum SensorPresenceBits : uint16_t {
    SENSOR_TEMP_VAPOUR_HEAD     = (1 << 0),
    SENSOR_TEMP_BOILER_LIQUID   = (1 << 1),
    SENSOR_TEMP_PCB_ENVIRONMENT = (1 << 2),
    SENSOR_PRESSURE_AMBIENT     = (1 << 3),
    SENSOR_PRESSURE_VAPOUR      = (1 << 4),
    SENSOR_FLOW                 = (1 << 5),
    SENSOR_VALVE_REFLUX         = (1 << 6),
    SENSOR_VALVE_PRODUCT        = (1 << 7),
    SENSOR_HEATER_1             = (1 << 8),
    SENSOR_HEATER_2             = (1 << 9),
};

class Controller {
public:
    Controller(const Config& config, TelemetryPublisher& publisher)
        : config_(config)
        , publisher_(publisher)
        , sensor_presence_(0)
        , last_telemetry_ms_(0)
        , telemetry_interval_ms_(200)
    {
        // Load sensor presence map from config at initialization
        sensor_presence_ = config_.GetSensorPresenceMap();
        
        // Preallocate telemetry buffer
        telemetry_buffer_.resize(sizeof(TelemetryMessage));
    }

    void Run() {
        while (running_) {
            auto tick_start = std::chrono::steady_clock::now();
            
            // 100ms control tick
            UpdateState();
            ExecuteControlLogic();
            
            // 200ms telemetry publish (non-blocking)
            uint64_t now_ms = GetTimestampMs();
            if (now_ms - last_telemetry_ms_ >= telemetry_interval_ms_) {
                PublishTelemetry(now_ms);
                last_telemetry_ms_ = now_ms;
            }
            
            // Sleep to maintain 100ms tick
            auto tick_end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tick_end - tick_start);
            if (elapsed.count() < 100) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100 - elapsed.count()));
            }
        }
    }

private:
    void PublishTelemetry(uint64_t timestamp_ms) {
        TelemetryMessage msg;
        std::memset(&msg, 0, sizeof(msg));
        
        msg.version = TelemetryMessage::VERSION;
        msg.timestamp_ms = timestamp_ms;
        msg.mode = static_cast<uint8_t>(current_mode_);
        
        // Populate temperatures (null if sensor not present)
        msg.temp_vapour_head = (sensor_presence_ & SENSOR_TEMP_VAPOUR_HEAD)
            ? static_cast<int16_t>(state_.temp_vapour_head_degc * 100)
            : INT16_MAX;
        
        msg.temp_boiler_liquid = (sensor_presence_ & SENSOR_TEMP_BOILER_LIQUID)
            ? static_cast<int16_t>(state_.temp_boiler_liquid_degc * 100)
            : INT16_MAX;
        
        msg.temp_pcb_environment = (sensor_presence_ & SENSOR_TEMP_PCB_ENVIRONMENT)
            ? static_cast<int16_t>(state_.temp_pcb_environment_degc * 100)
            : INT16_MAX;
        
        // Populate pressures
        msg.pressure_ambient = (sensor_presence_ & SENSOR_PRESSURE_AMBIENT)
            ? static_cast<int16_t>(state_.pressure_ambient_kpa * 100)
            : INT16_MAX;
        
        msg.pressure_vapour = (sensor_presence_ & SENSOR_PRESSURE_VAPOUR)
            ? static_cast<int16_t>(state_.pressure_vapour_kpa * 100)
            : INT16_MAX;
        
        // Flow rate
        msg.flow_ml_min = (sensor_presence_ & SENSOR_FLOW)
            ? static_cast<uint16_t>(state_.flow_ml_min * 10)
            : UINT16_MAX;
        
        // Valve positions
        msg.valve_reflux_control = (sensor_presence_ & SENSOR_VALVE_REFLUX)
            ? state_.valve_reflux_percent
            : UINT8_MAX;
        
        msg.valve_product_takeoff = (sensor_presence_ & SENSOR_VALVE_PRODUCT)
            ? state_.valve_product_percent
            : UINT8_MAX;
        
        // Heater outputs
        msg.heater_1 = (sensor_presence_ & SENSOR_HEATER_1)
            ? state_.heater_1_percent
            : UINT8_MAX;
        
        msg.heater_2 = (sensor_presence_ & SENSOR_HEATER_2)
            ? state_.heater_2_percent
            : UINT8_MAX;
        
        // Faults and presence map
        msg.faults = state_.fault_flags;
        msg.sensor_presence = sensor_presence_;
        
        // Copy to preallocated buffer and publish
        std::memcpy(telemetry_buffer_.data(), &msg, sizeof(msg));
        
        // Non-blocking publish; publisher handles disconnected clients internally
        if (!publisher_.Publish(telemetry_buffer_.data(), telemetry_buffer_.size())) {
            // Log but don't fail the control loop
            // Publisher will clean up dead connections on its own
        }
    }

    void UpdateState() {
        // Read sensors into state_
        // (implementation depends on hardware abstraction layer)
    }

    void ExecuteControlLogic() {
        // Core 100ms control loop
        // (implementation depends on process control requirements)
    }

    uint64_t GetTimestampMs() const {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

    const Config& config_;
    TelemetryPublisher& publisher_;
    
    uint16_t sensor_presence_;
    uint64_t last_telemetry_ms_;
    const uint64_t telemetry_interval_ms_;
    
    std::vector<uint8_t> telemetry_buffer_;  // Preallocated, no runtime alloc
    
    ProcessState state_;
    ProcessMode current_mode_;
    bool running_ = true;
};

} // namespace equilibria