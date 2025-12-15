"""TCP client for communicating with the Equilibria controller."""

import asyncio
import json
import logging
from typing import Optional, Callable, Dict, Any

logger = logging.getLogger(__name__)


# Protocol constants
PROTOCOL_VERSION = "v0"
DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 7002

# Message types
MSG_GET_TELEMETRY = "get_telemetry"
MSG_SET_MODE = "set_mode"
MSG_SET_TARGETS = "set_targets"
MSG_TELEMETRY = "telemetry"
MSG_ACK = "ack"

# Modes
MODE_IDLE = "IDLE"
MODE_ACTIVE = "ACTIVE"


class ControllerClient:
    """Async TCP client for the Equilibria controller IPC protocol v0."""
    
    def __init__(
        self,
        host: str = DEFAULT_HOST,
        port: int = DEFAULT_PORT,
        reconnect_delay: float = 5.0
    ):
        self.host = host
        self.port = port
        self.reconnect_delay = reconnect_delay
        
        self._reader: Optional[asyncio.StreamReader] = None
        self._writer: Optional[asyncio.StreamWriter] = None
        self._running = False
        self._telemetry_callback: Optional[Callable[[Dict[str, Any]], None]] = None
        self._ack_callback: Optional[Callable[[Dict[str, Any]], None]] = None
        
    def set_telemetry_callback(self, callback: Callable[[Dict[str, Any]], None]):
        """Set callback for telemetry messages."""
        self._telemetry_callback = callback
        
    def set_ack_callback(self, callback: Callable[[Dict[str, Any]], None]):
        """Set callback for ACK messages."""
        self._ack_callback = callback
        
    async def connect(self) -> bool:
        """Connect to the controller. Returns True on success."""
        try:
            self._reader, self._writer = await asyncio.open_connection(
                self.host, self.port
            )
            logger.info(f"[API] Connected to controller at {self.host}:{self.port}")
            return True
        except Exception as e:
            logger.error(f"[API] Failed to connect to controller: {e}")
            return False
            
    async def disconnect(self):
        """Disconnect from the controller."""
        if self._writer:
            try:
                self._writer.close()
                await self._writer.wait_closed()
            except Exception as e:
                logger.error(f"[API] Error during disconnect: {e}")
            finally:
                self._writer = None
                self._reader = None
                
    async def _send_message(self, msg_type: str, payload: Dict[str, Any]):
        """Send a message to the controller."""
        if not self._writer:
            logger.error("[API] Not connected to controller")
            return
            
        message = {
            "version": PROTOCOL_VERSION,
            "type": msg_type,
            "payload": payload
        }
        
        try:
            json_str = json.dumps(message) + "\n"
            self._writer.write(json_str.encode('utf-8'))
            await self._writer.drain()
            logger.debug(f"[API] Sent {msg_type} message")
        except Exception as e:
            logger.error(f"[API] Failed to send message: {e}")
            await self.disconnect()
            
    async def get_telemetry(self):
        """Request telemetry from the controller."""
        await self._send_message(MSG_GET_TELEMETRY, {})
        
    async def set_mode(self, mode: str):
        """Set the controller operating mode."""
        await self._send_message(MSG_SET_MODE, {"mode": mode})
        
    async def set_targets(
        self,
        target_abv: Optional[float] = None,
        target_flow: Optional[float] = None
    ):
        """Set process targets."""
        payload = {}
        if target_abv is not None:
            payload["target_abv"] = target_abv
        if target_flow is not None:
            payload["target_flow"] = target_flow
            
        await self._send_message(MSG_SET_TARGETS, payload)
        
    async def _handle_message(self, message: Dict[str, Any]):
        """Handle an incoming message from the controller."""
        try:
            msg_type = message.get("type")
            payload = message.get("payload", {})
            
            if msg_type == MSG_TELEMETRY:
                if self._telemetry_callback:
                    self._telemetry_callback(payload)
                    
            elif msg_type == MSG_ACK:
                if self._ack_callback:
                    self._ack_callback(payload)
                logger.info(
                    f"[API] ACK for {payload.get('command')}: "
                    f"{payload.get('status')} - {payload.get('message', '')}"
                )
                
            else:
                logger.warning(f"[API] Unknown message type: {msg_type}")
                
        except Exception as e:
            logger.error(f"[API] Error handling message: {e}")
            
    async def _receive_loop(self):
        """Main receive loop for processing incoming messages."""
        incomplete_data = ""
        
        while self._running and self._reader:
            try:
                data = await self._reader.read(4096)
                
                if not data:
                    # Connection closed
                    logger.warning("[API] Controller connection closed")
                    break
                    
                incomplete_data += data.decode('utf-8')
                
                # Process complete messages (newline-delimited)
                while '\n' in incomplete_data:
                    line, incomplete_data = incomplete_data.split('\n', 1)
                    
                    if line.strip():
                        try:
                            message = json.loads(line)
                            await self._handle_message(message)
                        except json.JSONDecodeError as e:
                            logger.error(f"[API] Failed to parse JSON: {e}")
                            
            except Exception as e:
                logger.error(f"[API] Error in receive loop: {e}")
                break
                
        await self.disconnect()
        
    async def run(self):
        """Run the client with automatic reconnection."""
        self._running = True
        
        while self._running:
            if await self.connect():
                await self._receive_loop()
                
            if self._running:
                logger.info(f"[API] Reconnecting in {self.reconnect_delay}s...")
                await asyncio.sleep(self.reconnect_delay)
                
    async def stop(self):
        """Stop the client."""
        self._running = False
        await self.disconnect()
