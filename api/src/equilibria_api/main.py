import asyncio
import json
import logging
import sys
from typing import Optional
import aiohttp
from aiohttp import web

"""
Equilibria API server.

Provides REST endpoints for commands/config and WebSocket streaming for telemetry.
Controller is the source of truth; API is a thin communication layer only.
"""



# Configure structured logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    stream=sys.stdout
)
logger = logging.getLogger('equilibria_api')


class ControllerClient:
    """
    Client interface to the controller process.
    
    This is a placeholder that should communicate with the actual C++ controller
    via IPC (socket, shared memory, or similar).
    """
    
    def __init__(self):
        self.latest_telemetry: Optional[dict] = None
        self._running = False
        self._task: Optional[asyncio.Task] = None
        self._telemetry_callbacks = []
    
    async def start(self):
        """Start controller client task."""
        if self._running:
            return
        self._running = True
        self._task = asyncio.create_task(self._telemetry_loop())
        logger.info("Controller client started")
    
    async def stop(self):
        """Stop controller client task."""
        self._running = False
        if self._task:
            self._task.cancel()
            try:
                await self._task
            except asyncio.CancelledError:
                pass
        logger.info("Controller client stopped")
    
    async def _telemetry_loop(self):
        """
        Background task that receives telemetry from controller.
        
        TODO: Replace with actual IPC mechanism (socket, shared memory, etc.)
        """
        try:
            while self._running:
                # Placeholder: would read from controller here
                await asyncio.sleep(0.1)  # 10Hz max
                
                # Simulate telemetry update
                # In production, this would come from controller
                telemetry = {
                    "timestamp": asyncio.get_event_loop().time(),
                    "state": "idle",
                    "sensors": {},
                    "actuators": {}
                }
                
                self.latest_telemetry = telemetry
                
                # Notify WebSocket clients
                for callback in self._telemetry_callbacks:
                    try:
                        await callback(telemetry)
                    except Exception as e:
                        logger.error(f"Error in telemetry callback: {e}")
        except asyncio.CancelledError:
            logger.info("Telemetry loop cancelled")
            raise
        except Exception as e:
            logger.error(f"Telemetry loop error: {e}")
            raise
    
    def subscribe_telemetry(self, callback):
        """Subscribe to telemetry updates."""
        self._telemetry_callbacks.append(callback)
    
    def unsubscribe_telemetry(self, callback):
        """Unsubscribe from telemetry updates."""
        if callback in self._telemetry_callbacks:
            self._telemetry_callbacks.remove(callback)
    
    async def send_command(self, command_type: str, payload: dict) -> dict:
        """
        Send command to controller.
        
        TODO: Replace with actual IPC to controller.
        """
        logger.info(f"Command received: type={command_type}, payload={payload}")
        
        # Validate command type
        valid_types = {'set_mode', 'set_targets', 'get_telemetry'}
        if command_type not in valid_types:
            raise ValueError(f"Invalid command type: {command_type}")
        
        # Placeholder response
        return {
            "status": "accepted",
            "command_type": command_type,
            "timestamp": asyncio.get_event_loop().time()
        }


async def health_handler(request: web.Request) -> web.Response:
    """GET /health - Health check endpoint."""
    return web.json_response({"status": "ok"})


async def telemetry_handler(request: web.Request) -> web.Response:
    """GET /telemetry - Return latest telemetry snapshot."""
    controller: ControllerClient = request.app['controller']
    
    if controller.latest_telemetry is None:
        return web.json_response(
            {"error": "No telemetry available yet"},
            status=503
        )
    
    return web.json_response(controller.latest_telemetry)


async def command_handler(request: web.Request) -> web.Response:
    """POST /command - Forward command to controller."""
    controller: ControllerClient = request.app['controller']
    
    try:
        body = await request.json()
    except json.JSONDecodeError:
        return web.json_response(
            {"error": "Invalid JSON"},
            status=400
        )
    
    command_type = body.get('type')
    payload = body.get('payload', {})
    
    if not command_type:
        return web.json_response(
            {"error": "Missing 'type' field"},
            status=400
        )
    
    try:
        result = await controller.send_command(command_type, payload)
        return web.json_response(result)
    except ValueError as e:
        return web.json_response(
            {"error": str(e)},
            status=400
        )
    except Exception as e:
        logger.error(f"Command error: {e}")
        return web.json_response(
            {"error": "Internal server error"},
            status=500
        )


async def websocket_handler(request: web.Request) -> web.WebSocketResponse:
    """WebSocket /ws - Stream telemetry updates to clients."""
    ws = web.WebSocketResponse()
    await ws.prepare(request)
    
    controller: ControllerClient = request.app['controller']
    
    logger.info(f"WebSocket client connected: {request.remote}")
    
    # Create queue for this client
    queue = asyncio.Queue(maxsize=10)
    
    async def telemetry_callback(telemetry: dict):
        """Push telemetry to client queue (non-blocking)."""
        try:
            queue.put_nowait(telemetry)
        except asyncio.QueueFull:
            # Drop old telemetry if client is slow
            try:
                queue.get_nowait()
                queue.put_nowait(telemetry)
            except (asyncio.QueueEmpty, asyncio.QueueFull):
                pass
    
    controller.subscribe_telemetry(telemetry_callback)
    
    try:
        # Send initial telemetry if available
        if controller.latest_telemetry:
            await ws.send_json(controller.latest_telemetry)
        
        # Process messages
        async def send_telemetry():
            while True:
                telemetry = await queue.get()
                await ws.send_json(telemetry)
        
        send_task = asyncio.create_task(send_telemetry())
        
        try:
            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.ERROR:
                    logger.warning(f"WebSocket error: {ws.exception()}")
                    break
        finally:
            send_task.cancel()
            try:
                await send_task
            except asyncio.CancelledError:
                pass
    finally:
        controller.unsubscribe_telemetry(telemetry_callback)
        logger.info(f"WebSocket client disconnected: {request.remote}")
    
    return ws


async def on_startup(app: web.Application):
    """Start controller client on app startup."""
    controller = ControllerClient()
    app['controller'] = controller
    await controller.start()
    logger.info("Application startup complete")


async def on_shutdown(app: web.Application):
    """Stop controller client on app shutdown."""
    controller: ControllerClient = app['controller']
    await controller.stop()
    logger.info("Application shutdown complete")


def create_app() -> web.Application:
    """Create and configure the aiohttp application."""
    app = web.Application()
    
    # Add routes
    app.router.add_get('/health', health_handler)
    app.router.add_get('/telemetry', telemetry_handler)
    app.router.add_post('/command', command_handler)
    app.router.add_get('/ws', websocket_handler)
    
    # Setup lifecycle handlers
    app.on_startup.append(on_startup)
    app.on_shutdown.append(on_shutdown)
    
    return app


def main():
    """Main entry point."""
    app = create_app()
    
    port = 7125
    logger.info(f"Starting Equilibria API on port {port}")
    
    web.run_app(app, host='0.0.0.0', port=port, access_log=logger)


if __name__ == '__main__':
    main()