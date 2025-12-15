# Equilibria – Copilot Instructions

You are assisting on Equilibria, a modular process-control platform for distillation and phase-change processes.

## Project Overview
Equilibria is designed with strict separation of concerns:
- **Controller**: Real-time process control logic (C++)
- **API**: HTTP/WebSocket gateway for UI and external systems (Python)
- **UI**: Web-based monitoring and control interface (Vue.js)
- **Shared**: Protocol definitions and utilities (C++)
- **Sim**: Test harnesses and MCU simulation (C++)

The system emphasizes safety, determinism, and modularity. All components communicate via versioned protocols.

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

## Development Workflow

### Building the project

**C++ modules (controller, sim, shared):**
```bash
# From the module directory (e.g., controller/)
mkdir -p build && cd build
cmake ..
make
```

**Python API:**
```bash
cd api/
pip install -e .  # Development install
# or
pip install .     # Regular install
```

**Vue UI:**
```bash
cd ui/
npm install       # Install dependencies
npm run dev       # Development server
npm run build     # Production build
```

### Running tests

**C++ tests:**
```bash
# From the build directory
ctest
# or
make test
```

**Python tests:**
```bash
cd api/
pytest                    # Run all tests
pytest -v                 # Verbose output
pytest tests/test_foo.py  # Run specific test file
```

**Vue tests:**
```bash
cd ui/
npm run test        # Run all tests
npm run test:unit   # Unit tests only
```

### Linting and formatting

**C++ (if clang-format is configured):**
```bash
clang-format -i **/*.cpp **/*.h
```

**Python:**
```bash
cd api/
black .              # Auto-format
ruff check .         # Lint
mypy .               # Type checking
```

**Vue/TypeScript:**
```bash
cd ui/
npm run lint         # Lint
npm run format       # Auto-format
```

### Running the application

**Controller:**
```bash
cd controller/build
./equilibria_controller --config ../config/machine.cfg
# Configuration files: machine.cfg, roles.cfg, safety.cfg, sensors.cfg
```

**API Server:**
```bash
cd api/
python -m equilibria_api.main
# or if installed:
equilibria-api
```

**UI (development):**
```bash
cd ui/
npm run dev
# Open browser to the URL displayed in the terminal (typically http://localhost:5173)
```

## File Organization Patterns

- **config/**: Configuration files (YAML/JSON)
- **docs/**: Architecture diagrams, design documents
- ***/src/**: Implementation source files
- ***/include/**: Public headers (C++ modules)
- ***/tests/**: Unit and integration tests
- ***/build/**: Build artifacts (gitignored)

## Dependencies and Environment

**C++ Requirements:**
- CMake 3.10+
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- System dependencies vary by module

**Python Requirements:**
- Python 3.8+
- Dependencies defined in `api/pyproject.toml`
- Key libraries: aiohttp, asyncio

**Node/Vue Requirements:**
- Node.js 16+
- Dependencies defined in `ui/package.json`
- Key frameworks: Vue 3, Vite

## Common Patterns

### Adding a new protocol message
1. Define in `shared/include/protocol.h` (or relevant header)
2. Add serialization/deserialization functions
3. Add unit tests in `shared/tests/`
4. Update protocol version
5. Document in API reference

### Adding a new API endpoint
1. Define route in the API module (`api/src/`)
2. Ensure no control logic—delegate to controller
3. Add request/response validation
4. Add unit tests
5. Update API documentation

### Adding a new UI component
1. Create component in `ui/src/components/`
2. Ensure no business logic—only presentation
3. Use props and events for data flow
4. Add component tests if applicable
5. Update relevant views

When uncertain, choose the simplest design that preserves these boundaries.
