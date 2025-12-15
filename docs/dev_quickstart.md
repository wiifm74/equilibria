# Developer Quickstart (Windows)

This guide helps you build and run Equilibria on Windows from a fresh checkout.

---

## Prerequisites

Install the following tools in order:

1. **Git for Windows**  
    [https://git-scm.com/download/win](https://git-scm.com/download/win)

2. **Visual Studio Code**  
    [https://code.visualstudio.com/](https://code.visualstudio.com/)

3. **CMake** (3.15 or newer)  
    [https://cmake.org/download/](https://cmake.org/download/)  
    ✓ Add CMake to PATH during install

4. **MSVC Build Tools**  
    Install via Visual Studio Installer:  
    - [Visual Studio 2022 Community](https://visualstudio.microsoft.com/) or Build Tools  
    - Select "Desktop development with C++"  
    - Includes MSVC compiler and Windows SDK

5. **Python 3.10+**  
    [https://www.python.org/downloads/](https://www.python.org/downloads/)  
    ✓ Add Python to PATH

6. **Node.js 18+ (LTS)**  
    [https://nodejs.org/](https://nodejs.org/)

---

## Clone and Setup

```powershell
git clone https://github.com/your-org/equilibria.git
cd equilibria
```

---

## Build C++ Components (controller, sim)

Open **Developer PowerShell for VS 2022** or **Developer Command Prompt**.

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build Release configuration
cmake --build . --config Release

cd ..
```

Binaries will be in `build/Release/`.

---

## Run the System

Open **four separate terminals** (PowerShell or CMD):

### Terminal 1: Fake MCU (Simulator)
```powershell
.\build\Release\fake_mcu.exe
```
Simulates sensor/actuator hardware. Listens on serial or TCP.

### Terminal 2: Controller
```powershell
.\build\Release\controller.exe --config config/default.toml
```
Runs the control loop. Connects to fake_mcu and exposes state.

### Terminal 3: API Server
```powershell
cd api
python -m venv venv
.\venv\Scripts\Activate.ps1
pip install -r requirements.txt
python server.py
```
REST + WebSocket API on `http://localhost:8000`.

### Terminal 4: UI Dev Server
```powershell
cd ui
npm install
npm run dev
```
Vue dev server on `http://localhost:5173`.

---

## Verify Telemetry

1. Open browser: `http://localhost:5173`
2. You should see live telemetry streaming (temperature, pressure, state)
3. Try sending a command (e.g., "Start Process") via the UI
4. Check controller terminal for state transitions

---

## Troubleshooting

### Port Already in Use
- **API (8000)**: Change port in `api/config.py`
- **UI (5173)**: Change in `ui/vite.config.js` or use `npm run dev -- --port 5174`
- **Controller/fake_mcu**: Check `config/default.toml` for serial/TCP port settings

### Windows Firewall Prompt
- Click **Allow Access** for `python.exe` and `node.exe`
- If blocked, go to Windows Defender Firewall → Allow an app

### CMake Fails to Detect Compiler
- Run from **Developer PowerShell/Command Prompt for VS 2022**
- OR: `cmake .. -G "Visual Studio 17 2022" -A x64 -T host=x64`

### Python venv Activation Restricted
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Build Errors in C++
- Ensure MSVC toolchain is selected (`-G "Visual Studio 17 2022"`)
- Check C++17 support: CMakeLists.txt should set `CMAKE_CXX_STANDARD 17`

---

## Next Steps

- **Config**: Edit `config/default.toml` to map roles to your hardware
- **Docs**: See `docs/architecture.md` for module boundaries
- **Tests**: Run `ctest` in `build/` directory for C++ unit tests
- **API**: See `api/README.md` for REST endpoints and WebSocket protocol
