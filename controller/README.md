# Equilibria Controller

The Equilibria controller is the authoritative control logic and system state manager for the Equilibria distillation system.

## Building

### Requirements

- CMake 3.10 or newer
- C++17 compatible compiler (GCC, Clang, MSVC)
- Windows: WinSock2 (included with Windows SDK)
- Linux/macOS: POSIX sockets (standard)

### Build Instructions

```bash
cd controller
mkdir build
cd build
cmake ..
make  # or cmake --build . on Windows
```

The executable will be created at `build/bin/equilibria-controller`.

## Running

Start the controller:

```bash
./build/bin/equilibria-controller
```

The controller will:
1. Start a TCP server on `127.0.0.1:7002`
2. Wait for client connections
3. Send telemetry at 10 Hz (every 100ms)
4. Accept commands: `get_telemetry`, `set_mode`, `set_targets`

## IPC Protocol

The controller implements the IPC v0 protocol for communication with the API server. See [docs/protocol.md](../docs/protocol.md) for full details.

### Supported Commands

- `get_telemetry` - Request current telemetry snapshot
- `set_mode` - Set operating mode (IDLE or ACTIVE)
- `set_targets` - Set process targets (target_abv, target_flow)

### Response Messages

- `telemetry` - Periodic state updates (10 Hz)
- `ack` - Command acknowledgements

## Architecture

The controller is responsible for:
- System state management
- Safety decisions
- Mode transitions
- Sensor data aggregation
- Actuator control
- Fault detection

The controller is the **single source of truth** for system state.
