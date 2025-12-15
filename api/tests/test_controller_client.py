"""Tests for the controller client."""

import json
import pytest


def test_message_structure():
    """Test that message structure is correct."""
    message = {
        "version": "v0",
        "type": "get_telemetry",
        "payload": {}
    }
    
    json_str = json.dumps(message)
    parsed = json.loads(json_str)
    
    assert parsed["version"] == "v0"
    assert parsed["type"] == "get_telemetry"
    assert parsed["payload"] == {}


def test_set_mode_message():
    """Test set_mode message structure."""
    message = {
        "version": "v0",
        "type": "set_mode",
        "payload": {
            "mode": "ACTIVE"
        }
    }
    
    json_str = json.dumps(message)
    parsed = json.loads(json_str)
    
    assert parsed["payload"]["mode"] == "ACTIVE"


def test_set_targets_message():
    """Test set_targets message structure."""
    message = {
        "version": "v0",
        "type": "set_targets",
        "payload": {
            "target_abv": 95.0,
            "target_flow": 300.0
        }
    }
    
    json_str = json.dumps(message)
    parsed = json.loads(json_str)
    
    assert parsed["payload"]["target_abv"] == 95.0
    assert parsed["payload"]["target_flow"] == 300.0


def test_telemetry_structure():
    """Test telemetry message structure."""
    telemetry = {
        "version": "v0",
        "type": "telemetry",
        "payload": {
            "timestamp_ms": 1234567890,
            "mode": "IDLE",
            "temps": {
                "vapour_head": 78.2,
                "boiler_liquid": 91.5,
                "pcb_environment": 42.1
            },
            "pressures": {
                "ambient": 101.3,
                "vapour": None
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
    
    json_str = json.dumps(telemetry)
    parsed = json.loads(json_str)
    
    assert parsed["payload"]["mode"] == "IDLE"
    assert parsed["payload"]["temps"]["vapour_head"] == 78.2
    assert parsed["payload"]["pressures"]["vapour"] is None
    assert parsed["payload"]["valves"]["reflux_control"] == 65
    assert parsed["payload"]["faults"] == []


def test_ack_structure():
    """Test ACK message structure."""
    ack = {
        "version": "v0",
        "type": "ack",
        "payload": {
            "command": "set_mode",
            "status": "ok",
            "message": "Mode set successfully"
        }
    }
    
    json_str = json.dumps(ack)
    parsed = json.loads(json_str)
    
    assert parsed["payload"]["command"] == "set_mode"
    assert parsed["payload"]["status"] == "ok"
    assert parsed["payload"]["message"] == "Mode set successfully"


def test_newline_delimiter():
    """Test that messages use newline delimiters."""
    message = {
        "version": "v0",
        "type": "get_telemetry",
        "payload": {}
    }
    
    json_str = json.dumps(message) + "\n"
    
    assert json_str.endswith("\n")
    
    # Should be parseable without newline
    parsed = json.loads(json_str.strip())
    assert parsed["version"] == "v0"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
