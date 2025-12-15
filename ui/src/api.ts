// api.ts - Minimal API helper for Equilibria UI
// Connects to Python API for commands and telemetry streaming

const API_BASE = 'http://127.0.0.1:7125';
const WS_BASE = 'ws://127.0.0.1:7125';

interface CommandPayload {
    [key: string]: any;
}

interface TelemetryMessage {
    type: string;
    timestamp: number;
    data: any;
}

/**
 * Send a command to the API
 */
export async function sendCommand(type: string, payload: CommandPayload = {}): Promise<void> {
    const response = await fetch(`${API_BASE}/command`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ type, payload }),
    });

    if (!response.ok) {
        const error = await response.text().catch(() => 'Unknown error');
        throw new Error(`Command failed (${response.status}): ${error}`);
    }
}

/**
 * Connect to telemetry WebSocket with automatic reconnection
 */
export function connectTelemetry(
    onTelemetry: (message: TelemetryMessage) => void,
    onError?: (error: Error) => void
): () => void {
    let ws: WebSocket | null = null;
    let reconnectAttempt = 0;
    let reconnectTimer: number | null = null;
    let intentionallyClosed = false;

    const connect = () => {
        if (intentionallyClosed) return;

        try {
            ws = new WebSocket(`${WS_BASE}/telemetry`);

            ws.onopen = () => {
                reconnectAttempt = 0;
            };

            ws.onmessage = (event) => {
                try {
                    const message: TelemetryMessage = JSON.parse(event.data);
                    onTelemetry(message);
                } catch (err) {
                    const error = new Error(`Failed to parse telemetry: ${err}`);
                    onError?.(error);
                }
            };

            ws.onerror = (event) => {
                const error = new Error('WebSocket error');
                onError?.(error);
            };

            ws.onclose = () => {
                if (intentionallyClosed) return;

                // Exponential backoff: 1s, 2s, 4s, 8s, max 30s
                const delay = Math.min(1000 * Math.pow(2, reconnectAttempt), 30000);
                reconnectAttempt++;

                reconnectTimer = window.setTimeout(connect, delay);
            };
        } catch (err) {
            const error = new Error(`Failed to connect: ${err}`);
            onError?.(error);

            // Retry on connection failure
            const delay = Math.min(1000 * Math.pow(2, reconnectAttempt), 30000);
            reconnectAttempt++;
            reconnectTimer = window.setTimeout(connect, delay);
        }
    };

    connect();

    // Return cleanup function
    return () => {
        intentionallyClosed = true;
        if (reconnectTimer !== null) {
            clearTimeout(reconnectTimer);
            reconnectTimer = null;
        }
        if (ws) {
            ws.close();
            ws = null;
        }
    };
}