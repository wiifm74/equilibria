import asyncio
import json
import logging
from typing import Optional, Callable, Awaitable, Any

"""
Asyncio TCP client for Equilibria controller IPC.

Connects to the controller at 127.0.0.1:7002 using newline-delimited JSON.
Handles auto-reconnect, command sending, and telemetry streaming.
"""


logger = logging.getLogger(__name__)


class ControllerClient:
    """
    Asyncio TCP client for controller IPC (newline-delimited JSON).
    
    Automatically reconnects on disconnect and maintains latest telemetry snapshot.
    """
    
    def __init__(
        self,
        host: str = "127.0.0.1",
        port: int = 7002,
        reconnect_delay: float = 2.0
    ):
        self.host = host
        self.port = port
        self.reconnect_delay = reconnect_delay
        
        self._reader: Optional[asyncio.StreamReader] = None
        self._writer: Optional[asyncio.StreamWriter] = None
        self._connected = False
        self._running = False
        self._read_task: Optional[asyncio.Task] = None
        
        # Latest telemetry snapshot
        self._telemetry: Optional[dict] = None
        
        # Telemetry update callback
        self._telemetry_callback: Optional[Callable[[dict], Awaitable[None]]] = None
    
    @property
    def connected(self) -> bool:
        """Check if currently connected to controller."""
        return self._connected
    
    @property
    def telemetry(self) -> Optional[dict]:
        """Get the latest telemetry snapshot."""
        return self._telemetry
    
    def set_telemetry_callback(self, callback: Callable[[dict], Awaitable[None]]) -> None:
        """Set async callback for telemetry updates."""
        self._telemetry_callback = callback
    
    async def start(self) -> None:
        """Start the client with auto-reconnect loop."""
        self._running = True
        while self._running:
            try:
                await self._connect()
                self._read_task = asyncio.create_task(self._read_loop())
                await self._read_task
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Connection error: {e}")
            
            self._connected = False
            if self._running:
                logger.info(f"Reconnecting in {self.reconnect_delay}s...")
                await asyncio.sleep(self.reconnect_delay)
    
    async def stop(self) -> None:
        """Stop the client and close connection."""
        self._running = False
        if self._read_task:
            self._read_task.cancel()
            try:
                await self._read_task
            except asyncio.CancelledError:
                pass
        await self._disconnect()
    
    async def _connect(self) -> None:
        """Establish TCP connection to controller."""
        logger.info(f"Connecting to controller at {self.host}:{self.port}...")
        self._reader, self._writer = await asyncio.open_connection(self.host, self.port)
        self._connected = True
        logger.info("Connected to controller")
    
    async def _disconnect(self) -> None:
        """Close TCP connection."""
        if self._writer:
            try:
                self._writer.close()
                await self._writer.wait_closed()
            except Exception as e:
                logger.debug(f"Error closing connection: {e}")
            finally:
                self._writer = None
                self._reader = None
    
    async def _read_loop(self) -> None:
        """Continuously read and process incoming messages."""
        while self._connected and self._reader:
            try:
                line = await self._reader.readline()
                if not line:
                    logger.warning("Controller closed connection")
                    break
                
                await self._handle_message(line.decode('utf-8').strip())
            except asyncio.CancelledError:
                raise
            except Exception as e:
                logger.error(f"Read error: {e}")
                break
    
    async def _handle_message(self, line: str) -> None:
        """Parse and handle a single JSON message."""
        if not line:
            return
        
        try:
            msg = json.loads(line)
        except json.JSONDecodeError as e:
            logger.warning(f"Invalid JSON from controller: {e}")
            return
        
        # Validate version
        version = msg.get("version")
        if version != "v0":
            logger.warning(f"Unexpected message version: {version}")
            return
        
        msg_type = msg.get("type")
        
        if msg_type == "telemetry":
            self._telemetry = msg
            if self._telemetry_callback:
                try:
                    await self._telemetry_callback(msg)
                except Exception as e:
                    logger.error(f"Telemetry callback error: {e}")
        elif msg_type is None:
            logger.warning("Message missing 'type' field")
        else:
            logger.debug(f"Ignoring unknown message type: {msg_type}")
    
    async def _send_message(self, msg: dict) -> None:
        """Send a JSON message to the controller."""
        if not self._connected or not self._writer:
            raise ConnectionError("Not connected to controller")
        
        msg["version"] = "v0"
        line = json.dumps(msg) + "\n"
        self._writer.write(line.encode('utf-8'))
        await self._writer.drain()
    
    async def get_telemetry(self) -> None:
        """Request telemetry snapshot from controller."""
        await self._send_message({"type": "get_telemetry"})
    
    async def set_mode(self, mode: str) -> None:
        """Set controller operating mode."""
        await self._send_message({"type": "set_mode", "mode": mode})
    
    async def set_targets(self, targets: dict[str, Any]) -> None:
        """Set target values for controller."""
        await self._send_message({"type": "set_targets", "targets": targets})