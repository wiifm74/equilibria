# Protocol Documentation: Controller ↔ API IPC v0

## Overview

The controller and API communicate over a local IPC channel using newline-delimited JSON over TCP.

- **Transport:** TCP on `localhost:5555` (configurable)
- **Framing:** Each message is a single-line JSON object terminated by `\n`
- **Versioning:** Every message includes a `version` field (currently `0`)
- **Direction:** Bi-directional; both controller and API can initiate messages

## Ownership Rules

- **Controller** owns all process state, configuration, and safety decisions
- **API** is a stateless relay: commands from clients → controller; telemetry from controller → clients
- **API** must not interpret, cache, or modify control logic
- **Controller** is authoritative; API failures must not compromise process safety

## Message Structure

All messages share:

```json
{
    "version": 0,
    "type": "message_type",
    ...
}
```

## Message Types

### 1. `get_telemetry` (API → Controller)

Request current telemetry snapshot.

```json
{
    "version": 0,
    "type": "get_telemetry",
    "request_id": "uuid-1234"
}
```

### 2. `telemetry` (Controller → API)

Periodic or on-demand telemetry broadcast.

```json
{
    "version": 0,
    "type": "telemetry",
    "timestamp": 1672531200,
    "mode": "auto",
    "sensors": {
        "temp_1": 23.5,
        "pressure_2": 101.3
    },
    "actuators": {
        "heater_1": 50.0,
        "valve_2": true
    },
    "targets": {
        "temp_1": 25.0
    }
}
```

### 3. `set_mode` (API → Controller)

Change operational mode (e.g., `manual`, `auto`, `safe`).

```json
{
    "version": 0,
    "type": "set_mode",
    "request_id": "uuid-5678",
    "mode": "auto"
}
```

### 4. `set_targets` (API → Controller)

Update setpoints for controlled variables.

```json
{
    "version": 0,
    "type": "set_targets",
    "request_id": "uuid-abcd",
    "targets": {
        "temp_1": 26.0,
        "pressure_2": 100.0
    }
}
```

### 5. `ack` (Controller → API)

Acknowledgment of a command with success/failure status.

```json
{
    "version": 0,
    "type": "ack",
    "request_id": "uuid-5678",
    "success": true,
    "message": "Mode changed to auto"
}
```

**On failure:**

```json
{
    "version": 0,
    "type": "ack",
    "request_id": "uuid-abcd",
    "success": false,
    "error": "invalid_target",
    "message": "Sensor 'temp_99' does not exist"
}
```

## Error Handling

- **Malformed JSON:** Close connection; controller logs error
- **Unknown `type`:** Controller sends `ack` with `success: false`, `error: "unknown_message_type"`
- **Missing `version`:** Reject; treat as protocol violation
- **Version mismatch:** Future; for now, only `version: 0` is valid
- **Invalid command parameters:** Return `ack` with `success: false` and descriptive `error` code

## Versioning Strategy

- Breaking changes increment `version`
- Controller must reject messages with unsupported versions
- Backward-compatible additions (new optional fields) do not require version bump
- All protocol changes require updates to tests and this document

## Connection Lifecycle

1. API connects to controller on startup
2. Controller accepts connection and begins telemetry broadcast (configurable interval, e.g., 100ms)
3. API forwards commands from REST/WebSocket clients to controller
4. Controller sends `ack` for each command with `request_id`
5. On disconnect, controller continues operating; API reconnects with backoff

## Implementation Notes

- **Controller:** Uses non-blocking I/O; processes messages outside the 100ms control loop
- **API:** Maintains WebSocket subscriptions; broadcasts telemetry to all connected clients
- **Testing:** Mock sockets for unit tests; integration tests validate full message round-trips
