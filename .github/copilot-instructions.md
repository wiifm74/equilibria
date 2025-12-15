# Equilibria – Copilot Instructions

You are assisting on Equilibria, a modular process-control platform.

## Architecture boundaries
- controller/ (C++): owns process logic, configuration semantics, role mapping, control loops
- api/ (Python): API + WebSocket streaming only; must not contain control logic
- ui/ (Vue): presentation only; must not contain business logic
- shared/ (C++): protocol contracts, CRC, config parsing utilities used by controller/sim
- sim/ (C++): simulation and test harnesses

## Key principles
- Safety and determinism first
- Prefer explicit state machines
- Configuration is declarative and role-based (no hard-coded sensor/actuator assumptions)
- All messages and schemas must be versioned
- Any change to a contract requires updating tests and docs

## C++ guidelines (controller, sim, shared)
- Target C++17 or newer
- Avoid dynamic allocation in the 100ms control loop
- Prefer value types, POD structs for protocol frames
- Keep module interfaces small and testable
- Use clear error handling; do not silently ignore failures
- Log important state transitions

## Python guidelines (api)
- Use asyncio and aiohttp
- REST is for commands/config changes; WebSocket is for telemetry streaming
- API must not implement process control logic or safety decisions
- Controller is the source of truth for state

## Vue guidelines (ui)
- UI talks only to the API
- UI is read-only for telemetry and provides user commands via API
- Do not encode control logic in the UI
- Keep components minimal and testable

## Testing expectations
- Protocol encode/decode must have unit tests
- Config parsing must fail loudly on invalid input
- Controllers must expose simulated/test entry points where possible

When uncertain, choose the simplest design that preserves these boundaries.
