#pragma once

#include <cstddef>
#include <cstdint>

namespace equilibria {

/**
 * @brief Non-blocking telemetry publisher for IPC
 * 
 * Publishes binary telemetry frames to connected clients.
 * Must not block or allocate memory in the control loop.
 */
class TelemetryPublisher {
public:
    TelemetryPublisher() = default;
    
    /**
     * @brief Initialize publisher (bind socket, etc.)
     * @return true if initialization succeeded
     */
    bool Initialize() {
        // TODO: Set up publishing mechanism (socket, shared memory, etc.)
        return true;
    }
    
    /**
     * @brief Publish telemetry frame to all connected clients
     * @param data Pointer to telemetry data (POD structure)
     * @param size Size of telemetry data in bytes
     * @return true if published successfully (non-blocking)
     * 
     * Note: This must be non-blocking. Disconnected clients are cleaned up
     * internally without affecting the control loop.
     */
    bool Publish(const uint8_t* data, size_t size) {
        // TODO: Implement non-blocking publish
        // For now, no-op (telemetry not yet implemented)
        (void)data;
        (void)size;
        return true;
    }
    
private:
    // Publisher state (socket handles, client list, etc.)
};

} // namespace equilibria
