#pragma once

#include <cstdint>

namespace equilibria {

/**
 * @brief Process operating modes
 */
enum class ProcessMode : uint8_t {
    IDLE = 0,
    STARTUP = 1,
    ACTIVE = 2,
    SHUTDOWN = 3,
    FAULT = 4
};

/**
 * @brief Real-time process state (sensor readings and actuator outputs)
 * 
 * POD structure updated at 100ms rate by the control loop.
 */
struct ProcessState {
    // Temperature readings (degC)
    float temp_vapour_head_degc = 0.0f;
    float temp_boiler_liquid_degc = 0.0f;
    float temp_pcb_environment_degc = 0.0f;
    
    // Pressure readings (kPa)
    float pressure_ambient_kpa = 0.0f;
    float pressure_vapour_kpa = 0.0f;
    
    // Flow rate (ml/min)
    float flow_ml_min = 0.0f;
    
    // Valve positions (0-100%)
    uint8_t valve_reflux_percent = 0;
    uint8_t valve_product_percent = 0;
    
    // Heater outputs (0-100%)
    uint8_t heater_1_percent = 0;
    uint8_t heater_2_percent = 0;
    
    // Fault flags bitfield
    uint32_t fault_flags = 0;
};

// Forward declarations
class Config;
class TelemetryPublisher;

} // namespace equilibria
