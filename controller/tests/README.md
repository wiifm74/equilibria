# Controller Tests

Unit tests for the controller IPC protocol and related functionality.

## Running Tests

### Build and run
```bash
cd controller/tests
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

Or run the test executable directly:
```bash
./test_ipc_protocol
```

### Windows
```powershell
cd controller\tests
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Release\test_ipc_protocol.exe
```

## Test Coverage

### IPC Protocol Tests (`test_ipc_protocol.cpp`)
- **Valid JSON**: Proper message parsing for all message types
- **Invalid JSON**: Malformed JSON, empty strings, wrong types
- **Version validation**: Missing version, unknown versions, invalid types
- **Missing fields**: Missing type, payload, or required payload fields
- **set_mode validation**: Missing mode, invalid mode type, invalid mode value
- **set_targets validation**: Missing fields, invalid types, range checking
- **State persistence**: Multiple commands, target updates

## Test Framework

Uses a lightweight custom test framework (`test_framework.hpp`) with:
- Simple `TEST()` macro for test definition
- Assertion macros: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_CONTAINS`
- Minimal overhead, fast execution
- CI-friendly output

## Adding New Tests

```cpp
#include "test_framework.hpp"

TEST(my_new_test) {
    // Setup
    IPCProtocol protocol;
    
    // Execute
    std::string response = protocol.process_message("...");
    
    // Verify
    ASSERT_TRUE(is_ok_ack(response));
}
```

Tests are automatically registered and will run with the test runner.
