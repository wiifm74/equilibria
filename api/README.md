# Equilibria API

The Equilibria API is a Python-based server that provides REST API and WebSocket bridge for communicating with the controller.

## Installation

### Requirements

- Python 3.8 or newer
- pip

### Install

```bash
cd api
pip install -e .
```

For development with tests:

```bash
pip install -e ".[dev]"
```

## Running

Start the API server:

```bash
equilibria-api
# or
python -m equilibria_api.main
```

The API will:
1. Connect to the controller at `127.0.0.1:7002`
2. Automatically reconnect if connection is lost
3. Receive telemetry at 10 Hz
4. Send commands to the controller

## Development

### Running Tests

```bash
cd api
python -m pytest tests/ -v
```

### Project Structure

```
api/
├── src/equilibria_api/
│   ├── __init__.py
│   ├── main.py              # Main entry point and demo
│   └── controller_client.py # TCP client for controller IPC
├── tests/
│   └── test_controller_client.py
└── pyproject.toml
```

## IPC Protocol

The API client implements the IPC v0 protocol for communication with the controller. See [docs/protocol.md](../docs/protocol.md) for full details.

### Commands

The API can send the following commands to the controller:

- `get_telemetry()` - Request current telemetry
- `set_mode(mode)` - Set operating mode ("IDLE" or "ACTIVE")
- `set_targets(target_abv, target_flow)` - Set process targets

### Receiving Messages

The client receives:

- `telemetry` messages (10 Hz periodic updates)
- `ack` messages (command acknowledgements)

### Callbacks

Set callbacks to handle incoming messages:

```python
from equilibria_api.controller_client import ControllerClient

client = ControllerClient()

def on_telemetry(payload):
    print(f"Mode: {payload['mode']}")
    print(f"Temps: {payload['temps']}")

def on_ack(payload):
    print(f"Command {payload['command']}: {payload['status']}")

client.set_telemetry_callback(on_telemetry)
client.set_ack_callback(on_ack)

# Start client
await client.run()
```

## Architecture

The API is responsible for:
- Maintaining connection to controller
- Translating REST/WebSocket to controller IPC
- Streaming telemetry to UI clients
- Command validation (future)

The API **must not** contain control logic or make safety decisions. The controller is the single source of truth.
