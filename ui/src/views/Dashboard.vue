<template>
    <div class="dashboard">
        <header class="dashboard-header">
            <h1>Equilibria Dashboard</h1>
            <div class="connection-status" :class="{ connected: isConnected }">
                {{ isConnected ? 'Connected' : 'Disconnected' }}
            </div>
        </header>

        <section class="controls">
            <h2>Controls</h2>
            <div class="control-buttons">
                <button @click="setMode('ACTIVE')" :disabled="!isConnected">Start</button>
                <button @click="setMode('IDLE')" :disabled="!isConnected">Stop</button>
            </div>
            <div class="control-inputs">
                <label>
                    Target ABV:
                    <input v-model.number="targetABV" type="number" step="0.1" min="0" max="100" />
                    <button @click="setTargets" :disabled="!isConnected">Set Targets</button>
                </label>
                <label>
                    Target Flow (mL/min):
                    <input v-model.number="targetFlow" type="number" step="1" min="0" />
                </label>
            </div>
        </section>

        <section class="telemetry">
            <h2>Telemetry</h2>
            <div class="telemetry-grid">
                <div class="telemetry-item">
                    <span class="label">Mode:</span>
                    <span class="value">{{ formatValue(telemetry.mode) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Vapour Head Temp (°C):</span>
                    <span class="value">{{ formatValue(telemetry.vapour_head_temp) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Boiler Liquid Temp (°C):</span>
                    <span class="value">{{ formatValue(telemetry.boiler_liquid_temp) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">PCB Temp (°C):</span>
                    <span class="value">{{ formatValue(telemetry.pcb_temp) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Ambient Pressure (Pa):</span>
                    <span class="value">{{ formatValue(telemetry.ambient_pressure) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Vapour Pressure (Pa):</span>
                    <span class="value">{{ formatValue(telemetry.vapour_pressure) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Flow (mL/min):</span>
                    <span class="value">{{ formatValue(telemetry.flow_ml_min) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Valve Positions:</span>
                    <span class="value">{{ formatValue(telemetry.valve_positions) }}</span>
                </div>
                <div class="telemetry-item">
                    <span class="label">Heater Outputs:</span>
                    <span class="value">{{ formatValue(telemetry.heater_outputs) }}</span>
                </div>
                <div class="telemetry-item fault" v-if="telemetry.faults">
                    <span class="label">Faults:</span>
                    <span class="value">{{ formatValue(telemetry.faults) }}</span>
                </div>
            </div>
        </section>
    </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'

const WS_URL = 'ws://127.0.0.1:7125/ws'
const API_URL = 'http://127.0.0.1:7125'

const isConnected = ref(false)
const telemetry = ref({
    mode: null,
    vapour_head_temp: null,
    boiler_liquid_temp: null,
    pcb_temp: null,
    ambient_pressure: null,
    vapour_pressure: null,
    flow_ml_min: null,
    valve_positions: null,
    heater_outputs: null,
    faults: null
})

const targetABV = ref(40)
const targetFlow = ref(100)

let ws = null
let reconnectTimer = null

const formatValue = (value) => {
    if (value === null || value === undefined) {
        return '—'
    }
    if (typeof value === 'number') {
        return value.toFixed(2)
    }
    return value
}

const connectWebSocket = () => {
    ws = new WebSocket(WS_URL)

    ws.onopen = () => {
        isConnected.value = true
        console.log('WebSocket connected')
    }

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data)
            Object.assign(telemetry.value, data)
        } catch (error) {
            console.error('Failed to parse telemetry:', error)
        }
    }

    ws.onclose = () => {
        isConnected.value = false
        console.log('WebSocket disconnected')
        reconnectTimer = setTimeout(connectWebSocket, 3000)
    }

    ws.onerror = (error) => {
        console.error('WebSocket error:', error)
    }
}

const setMode = async (mode) => {
    try {
        const response = await fetch(`${API_URL}/command`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command: 'set_mode', mode })
        })
        if (!response.ok) {
            console.error('Failed to set mode:', await response.text())
        }
    } catch (error) {
        console.error('Error setting mode:', error)
    }
}

const setTargets = async () => {
    try {
        const response = await fetch(`${API_URL}/command`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                command: 'set_targets',
                target_abv: targetABV.value,
                target_flow_ml_min: targetFlow.value
            })
        })
        if (!response.ok) {
            console.error('Failed to set targets:', await response.text())
        }
    } catch (error) {
        console.error('Error setting targets:', error)
    }
}

onMounted(() => {
    connectWebSocket()
})

onUnmounted(() => {
    if (reconnectTimer) {
        clearTimeout(reconnectTimer)
    }
    if (ws) {
        ws.close()
    }
})
</script>

<style scoped>
.dashboard {
    padding: 2rem;
    max-width: 1200px;
    margin: 0 auto;
}

.dashboard-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 2rem;
}

.connection-status {
    padding: 0.5rem 1rem;
    border-radius: 4px;
    background: #e74c3c;
    color: white;
    font-weight: 500;
}

.connection-status.connected {
    background: #27ae60;
}

.controls {
    margin-bottom: 2rem;
    padding: 1.5rem;
    background: #f8f9fa;
    border-radius: 8px;
}

.control-buttons {
    display: flex;
    gap: 1rem;
    margin-bottom: 1rem;
}

.control-buttons button {
    padding: 0.75rem 1.5rem;
    font-size: 1rem;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    background: #3498db;
    color: white;
}

.control-buttons button:disabled {
    background: #95a5a6;
    cursor: not-allowed;
}

.control-inputs {
    display: flex;
    gap: 2rem;
    align-items: flex-end;
}

.control-inputs label {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
}

.control-inputs input {
    padding: 0.5rem;
    border: 1px solid #ddd;
    border-radius: 4px;
    width: 150px;
}

.control-inputs button {
    padding: 0.5rem 1rem;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    background: #2ecc71;
    color: white;
}

.control-inputs button:disabled {
    background: #95a5a6;
    cursor: not-allowed;
}

.telemetry {
    padding: 1.5rem;
    background: #ffffff;
    border: 1px solid #e1e8ed;
    border-radius: 8px;
}

.telemetry-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 1rem;
}

.telemetry-item {
    display: flex;
    justify-content: space-between;
    padding: 0.75rem;
    background: #f8f9fa;
    border-radius: 4px;
}

.telemetry-item.fault {
    background: #ffe5e5;
    border: 1px solid #e74c3c;
}

.label {
    font-weight: 500;
    color: #2c3e50;
}

.value {
    font-family: 'Courier New', monospace;
    color: #34495e;
}
</style>