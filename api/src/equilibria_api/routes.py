from aiohttp import web
import logging

"""
API routes for Equilibria process control platform.
"""

logger = logging.getLogger(__name__)

# Allowed command types
ALLOWED_COMMANDS = {"set_mode", "set_targets", "get_telemetry"}

# Command payload schemas (shape and type validation only)
COMMAND_SCHEMAS = {
    "set_mode": {
        "required": ["mode"],
        "types": {"mode": str}
    },
    "set_targets": {
        "required": ["targets"],
        "types": {"targets": dict}
    },
    "get_telemetry": {
        "required": [],
        "types": {}
    }
}


def validate_command(data):
    """
    Validate command shape and types. Does not enforce business logic.
    
    Returns:
        tuple: (is_valid: bool, error_message: str or None)
    """
    if not isinstance(data, dict):
        return False, "Request body must be a JSON object"
    
    command_type = data.get("command")
    
    if not command_type:
        return False, "Missing 'command' field"
    
    if not isinstance(command_type, str):
        return False, "'command' must be a string"
    
    if command_type not in ALLOWED_COMMANDS:
        return False, f"Unknown command type: {command_type}. Allowed: {', '.join(ALLOWED_COMMANDS)}"
    
    schema = COMMAND_SCHEMAS[command_type]
    payload = data.get("payload", {})
    
    if not isinstance(payload, dict):
        return False, "'payload' must be an object"
    
    # Check required fields
    for field in schema["required"]:
        if field not in payload:
            return False, f"Missing required field in payload: {field}"
    
    # Check field types
    for field, expected_type in schema["types"].items():
        if field in payload and not isinstance(payload[field], expected_type):
            return False, f"Field '{field}' must be of type {expected_type.__name__}"
    
    return True, None


async def handle_command(request):
    """
    POST /command endpoint.
    
    Validates command shape/types and forwards to controller.
    """
    try:
        data = await request.json()
    except Exception as e:
        logger.warning(f"Invalid JSON in command request: {e}")
        return web.json_response(
            {"error": "Invalid JSON"},
            status=400
        )
    
    # Validate command structure and types
    is_valid, error_msg = validate_command(data)
    if not is_valid:
        logger.warning(f"Command validation failed: {error_msg}")
        return web.json_response(
            {"error": error_msg},
            status=400
        )
    
    # Forward to controller client
    controller_client = request.app.get("controller_client")
    if not controller_client:
        logger.error("Controller client not configured")
        return web.json_response(
            {"error": "Controller not available"},
            status=503
        )
    
    try:
        # Send command and await acknowledgment
        result = await controller_client.send_command(
            command=data["command"],
            payload=data.get("payload", {})
        )
        
        return web.json_response({
            "status": "ok",
            "result": result
        })
    
    except Exception as e:
        logger.error(f"Controller command failed: {e}")
        return web.json_response(
            {"error": "Controller command failed", "details": str(e)},
            status=500
        )


def setup_routes(app):
    """Register API routes."""
    app.router.add_post("/command", handle_command)