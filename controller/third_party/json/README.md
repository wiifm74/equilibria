# JSON Library for Controller

## Recommended: nlohmann/json

We use [nlohmann/json](https://github.com/nlohmann/json) - a modern, single-header C++11 JSON library.

**License:** MIT  
**Version:** 3.11.3 or later  
**Header:** `json.hpp`

## Installation

### Option 1: Download directly
```powershell
# Download the single header file
Invoke-WebRequest -Uri "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" -OutFile "json.hpp"
```

### Option 2: Using CMake FetchContent (recommended for production)
Add to your CMakeLists.txt:
```cmake
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)
target_link_libraries(controller PRIVATE nlohmann_json::nlohmann_json)
```

### Option 3: Git submodule
```bash
git submodule add https://github.com/nlohmann/json.git controller/third_party/json/nlohmann
```

## Usage Example

```cpp
#include "third_party/json/json.hpp"
using json = nlohmann::json;

// Parse
std::string message = R"({"type":"command","action":"start"})";
json j = json::parse(message);
std::string type = j["type"];

// Create
json response;
response["status"] = "ok";
response["value"] = 42.5;
std::string msg = response.dump(); // Serialize
```

## Why nlohmann/json?

- **Single header**: Easy to vendor and integrate
- **Modern C++**: Uses C++11/17 features naturally
- **Well-tested**: Industry standard with extensive test coverage
- **Intuitive API**: Natural C++ syntax
- **Header-only**: No linking required
- **MIT License**: Permissive for commercial use

## License

MIT License - see LICENSE file in this directory.

The library is maintained by Niels Lohmann and contributors.
Full license: https://github.com/nlohmann/json/blob/develop/LICENSE.MIT
