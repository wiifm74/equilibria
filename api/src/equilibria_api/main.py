"""Main entry point for the Equilibria API server."""

import asyncio
import logging
from typing import Dict, Any
from .controller_client import ControllerClient

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class APIServer:
    """Simple API server that demonstrates the IPC protocol."""
    
    def __init__(self):
        self.controller = ControllerClient()
        self.latest_telemetry: Dict[str, Any] = {}
        
        # Set up callbacks
        self.controller.set_telemetry_callback(self._on_telemetry)
        self.controller.set_ack_callback(self._on_ack)
        
    def _on_telemetry(self, payload: Dict[str, Any]):
        """Handle telemetry updates from controller."""
        self.latest_telemetry = payload
        logger.info(
            f"[API] Telemetry: mode={payload.get('mode')}, "
            f"timestamp={payload.get('timestamp_ms')}"
        )
        
    def _on_ack(self, payload: Dict[str, Any]):
        """Handle ACK responses from controller."""
        logger.info(
            f"[API] ACK received: {payload.get('command')} -> "
            f"{payload.get('status')}: {payload.get('message', '')}"
        )
        
    async def run(self):
        """Run the API server."""
        logger.info("[API] Starting Equilibria API (IPC v0 client)")
        
        # Start controller client in background
        client_task = asyncio.create_task(self.controller.run())
        
        # Wait for connection
        await asyncio.sleep(2)
        
        # Demo: Send some commands
        logger.info("[API] Sending test commands...")
        
        await asyncio.sleep(1)
        await self.controller.get_telemetry()
        
        await asyncio.sleep(2)
        await self.controller.set_mode("ACTIVE")
        
        await asyncio.sleep(2)
        await self.controller.set_targets(target_abv=95.0, target_flow=300.0)
        
        await asyncio.sleep(2)
        await self.controller.set_mode("IDLE")
        
        # Keep running to receive telemetry
        logger.info("[API] Listening for telemetry... (Ctrl+C to exit)")
        try:
            await client_task
        except KeyboardInterrupt:
            logger.info("[API] Shutting down...")
            await self.controller.stop()


def main():
    """Main entry point."""
    server = APIServer()
    
    try:
        asyncio.run(server.run())
    except KeyboardInterrupt:
        logger.info("[API] Interrupted by user")


if __name__ == "__main__":
    main()
