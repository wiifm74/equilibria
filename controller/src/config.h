#pragma once

#include <cstdint>
#include <string>

namespace equilibria {

/**
 * @brief Configuration loader and holder
 * 
 * Loads role-based configuration from files:
 * - machine.cfg: Hardware mappings
 * - roles.cfg: Role definitions (sensor/actuator assignments)
 * - safety.cfg: Safety limits and interlocks
 * - sensors.cfg: Sensor calibration data
 */
class Config {
public:
    Config() = default;
    
    /**
     * @brief Load configuration from directory
     * @param config_dir Path to directory containing .cfg files
     * @return true if all required configs loaded successfully
     */
    bool Load(const std::string& config_dir) {
        // TODO: Implement configuration loading
        // For now, return default configuration
        return true;
    }
    
    /**
     * @brief Get sensor presence map from configuration
     * @return Bitfield indicating which sensors are present
     */
    uint16_t GetSensorPresenceMap() const {
        // TODO: Read from loaded configuration
        // For now, assume all sensors present
        return 0xFFFF;
    }
    
private:
    // Configuration data structures
    // TODO: Add role mappings, limits, calibrations
};

} // namespace equilibria
