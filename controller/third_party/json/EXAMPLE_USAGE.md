# IPC Message Handler Example

This example shows how to integrate JSON parsing with the IPC server.

## Using nlohmann/json (Recommended)

```cpp
#include "ipc_server.hpp"
#include "third_party/json/json.hpp"

using json = nlohmann::json;
using namespace equilibria;

int main() {
    IpcServer server;
    
    server.start([](const std::string& message) {
        try {
            // Parse incoming JSON
            json j = json::parse(message);
            
            std::string type = j.value("type", "");
            
            if (type == "command") {
                std::string action = j.value("action", "");
                std::cout << "Command received: " << action << std::endl;
                
                // Process command...
                
                // Send response
                json response;
                response["type"] = "response";
                response["status"] = "ok";
                response["action"] = action;
                
                // Note: server.send() would need to be accessible here
                // In practice, you'd use a message queue or callback
                
            } else if (type == "query") {
                std::string field = j.value("field", "");
                std::cout << "Query received for: " << field << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    });
    
    // Keep running
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

## Using minimal_json (Zero dependencies)

```cpp
#include "ipc_server.hpp"
#include "third_party/json/minimal_json.hpp"

using namespace equilibria;
using namespace equilibria::minimal_json;

int main() {
    IpcServer server;
    
    server.start([](const std::string& message) {
        try {
            // Parse incoming JSON
            JsonObject j = JsonObject::parse(message);
            
            std::string type = j.getString("type");
            
            if (type == "command") {
                std::string action = j.getString("action");
                std::cout << "Command received: " << action << std::endl;
                
                // Send response
                JsonObject response;
                response.set("type", "response");
                response.set("status", "ok");
                response.set("action", action);
                
                std::string responseStr = response.dump();
                // Send via server...
            }
            
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    });
    
    return 0;
}
```

## Message Format

### Command
```json
{"type":"command","action":"start","params":{"target":42.5}}
```

### Query
```json
{"type":"query","field":"temperature"}
```

### Response
```json
{"type":"response","status":"ok","value":42.5}
```

### Telemetry
```json
{"type":"telemetry","timestamp":1234567890,"sensors":{"T1":65.2,"P1":101.3}}
```
