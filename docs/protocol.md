# Equilibria IPC Protocol v0

## Overview

This document defines the **v0 IPC protocol** between the Equilibria controller (C++) and API (Python) processes.

## Transport

- **Protocol**: TCP
- **Default address**: `127.0.0.1:7002`
- **Server**: Controller (C++)
- **Client**: API (Python)

## Message Framing

- One JSON object per message
- UTF-8 encoded
- Newline-delimited (`\n`) framing for simplicity

## Message Structure

Every message MUST include:

```json
{
  "version": "v0",
  "type": "...",
  "payload": { ... }
}
```

- `version` (string, required): Protocol version identifier
- `type` (string, required): Message type identifier
- `payload` (object, required): Message-specific data

## Message Types

### API → Controller

#### `get_telemetry`

Request a snapshot of current controller state.

```json
{
  "version": "v0",
  "type": "get_telemetry",
  "payload": {}
}
```

**Response**: `telemetry` message

---

#### `set_mode`

Set controller operating mode.

```json
{
  "version": "v0",
  "type": "set_mode",
  "payload": {
    "mode": "IDLE"
  }
}
```

**Payload fields**:
- `mode` (string, required): Operating mode. Valid values: `"IDLE"`, `"ACTIVE"`

**Response**: `ack` message

---

#### `set_targets`

Set process targets.

```json
{
  "version": "v0",
  "type": "set_targets",
  "payload": {
    "target_abv": 92.0,
    "target_flow": 250.0
  }
}
```

**Payload fields**:
- `target_abv` (number, optional): Target alcohol by volume percentage
- `target_flow` (number, optional): Target flow rate in mL/min

All fields are optional. The controller retains previous values if fields are omitted.

**Response**: `ack` message

---

### Controller → API

#### `telemetry`

Emitted periodically (e.g., 5–10 Hz) and/or in response to `get_telemetry`.

```json
{
  "version": "v0",
  "type": "telemetry",
  "payload": {
    "timestamp_ms": 123456789,
    "mode": "IDLE",
    "temps": {
      "vapour_head": 78.2,
      "boiler_liquid": 91.5,
      "pcb_environment": 42.1
    },
    "pressures": {
      "ambient": 101.3,
      "vapour": null
    },
    "flow_ml_min": 240.0,
    "valves": {
      "reflux_control": 65,
      "product_takeoff": 30
    },
    "heaters": {
      "heater_1": 70,
      "heater_2": 70
    },
    "faults": []
  }
}
```

**Payload fields**:
- `timestamp_ms` (number, required): Unix timestamp in milliseconds
- `mode` (string, required): Current operating mode (`"IDLE"` or `"ACTIVE"`)
- `temps` (object, required): Temperature readings in °C
  - `vapour_head` (number|null): Vapour head temperature
  - `boiler_liquid` (number|null): Boiler liquid temperature
  - `pcb_environment` (number|null): PCB environment temperature
- `pressures` (object, required): Pressure readings in kPa
  - `ambient` (number|null): Ambient pressure
  - `vapour` (number|null): Vapour pressure
- `flow_ml_min` (number|null, required): Flow rate in mL/min
- `valves` (object, required): Valve positions (0-100%)
  - `reflux_control` (number): Reflux control valve position
  - `product_takeoff` (number): Product takeoff valve position
- `heaters` (object, required): Heater power levels (0-100%)
  - `heater_1` (number): Heater 1 power level
  - `heater_2` (number): Heater 2 power level
- `faults` (array, required): List of active fault codes (strings)

**Note**: Missing or non-present sensors MUST be reported as `null`.

---

#### `ack`

Acknowledgement of a command.

```json
{
  "version": "v0",
  "type": "ack",
  "payload": {
    "command": "set_targets",
    "status": "ok",
    "message": "Targets updated successfully"
  }
}
```

**Payload fields**:
- `command` (string, required): The command type being acknowledged
- `status` (string, required): Status of command execution. Valid values: `"ok"`, `"error"`
- `message` (string, optional): Human-readable message providing additional context

---

## Ownership Rules

### Controller Responsibilities

The controller is the **single source of truth** for:
- System state
- Safety decisions
- Mode transitions
- Validity of commands

### API Responsibilities

The API must:
- Never infer or enforce control logic
- Treat controller responses as authoritative
- Derive UI state exclusively from telemetry

## Error Handling

| Error Condition | Behavior |
|----------------|----------|
| Invalid JSON | Controller closes connection |
| Unsupported version | Controller replies with `ack` error |
| Invalid command payload | Controller replies with `ack` error, no state change |
| Controller disconnect | API must reconnect |

## Versioning

The protocol version is indicated by the `version` field in every message.

- **Current version**: `v0`
- Version changes indicate breaking changes to the protocol
- Clients MUST check the version field and reject unsupported versions

## Example Message Flow

1. API connects to controller
2. Controller begins sending periodic `telemetry` messages (5-10 Hz)
3. API sends `get_telemetry` request
4. Controller responds with `telemetry` message
5. API sends `set_mode` request
6. Controller responds with `ack` message
7. Controller continues sending periodic `telemetry` messages with updated mode

## Security Considerations

- **v0 is intended for localhost only**
- No authentication mechanism in v0
- No encryption in v0
- Future versions may add authentication and TLS support

## Future Considerations

- Binary protocol for improved performance
- Message compression
- Authentication and encryption
- Unix domain sockets (for non-Windows platforms)
- gRPC/Protobuf (for production deployments)

## Changelog

### v0 (Initial Release)
- Initial protocol definition
- Basic command set: `get_telemetry`, `set_mode`, `set_targets`
- Basic response set: `telemetry`, `ack`
- Newline-delimited JSON framing
