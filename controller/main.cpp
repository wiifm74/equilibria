#include "../shared/include/ipc_protocol.hpp"
#include "../shared/include/json_utils.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
    #define SOCKET_ERROR_VAL SOCKET_ERROR
    #define INVALID_SOCKET_VAL INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSE_SOCKET close
    #define SOCKET_ERROR_VAL -1
    #define INVALID_SOCKET_VAL -1
#endif

using namespace equilibria;

// Controller state
struct ControllerState {
    std::atomic<bool> running{true};
    std::string mode{ipc::Mode::IDLE};
    double target_abv{92.0};
    double target_flow{250.0};
    
    // Simulated sensor readings
    ipc::TelemetryPayload get_telemetry() {
        ipc::TelemetryPayload telemetry;
        
        telemetry.timestamp_ms = ipc::get_timestamp_ms();
        telemetry.mode = mode;
        
        // Simulated temperature readings
        telemetry.temps.vapour_head = 78.2;
        telemetry.temps.boiler_liquid = 91.5;
        telemetry.temps.pcb_environment = 42.1;
        
        // Simulated pressure readings
        telemetry.pressures.ambient = 101.3;
        telemetry.pressures.vapour = std::nullopt; // Not present
        
        // Simulated flow
        telemetry.flow_ml_min = 240.0;
        
        // Valve positions
        telemetry.valves.reflux_control = 65;
        telemetry.valves.product_takeoff = 30;
        
        // Heater levels
        telemetry.heaters.heater_1 = 70;
        telemetry.heaters.heater_2 = 70;
        
        // No faults
        telemetry.faults = {};
        
        return telemetry;
    }
};

// Send a message to a client
bool send_message(socket_t client_socket, const std::string& message) {
#ifdef _WIN32
    int sent = send(client_socket, message.c_str(), static_cast<int>(message.length()), 0);
#else
    ssize_t sent = send(client_socket, message.c_str(), message.length(), 0);
#endif
    return sent > 0;
}

// Process incoming command
void process_command(socket_t client_socket, ControllerState& state, const ipc::Message& msg) {
    ipc::AckPayload ack;
    ack.command = msg.type;
    
    // Check version
    if (msg.version != ipc::PROTOCOL_VERSION) {
        ack.status = ipc::AckStatus::ERROR;
        ack.message = "Unsupported protocol version: " + msg.version;
        
        std::string response = json::create_ack_message(ack);
        send_message(client_socket, response);
        return;
    }
    
    // Process command
    if (msg.type == ipc::MessageType::GET_TELEMETRY) {
        // Send telemetry immediately
        auto telemetry = state.get_telemetry();
        std::string response = json::create_telemetry_message(telemetry);
        send_message(client_socket, response);
        
    } else if (msg.type == ipc::MessageType::SET_MODE) {
        auto payload = json::parse_set_mode(msg.payload_json);
        if (!payload.has_value()) {
            ack.status = ipc::AckStatus::ERROR;
            ack.message = "Invalid set_mode payload";
        } else {
            if (payload->mode == ipc::Mode::IDLE || payload->mode == ipc::Mode::ACTIVE) {
                state.mode = payload->mode;
                ack.status = ipc::AckStatus::OK;
                ack.message = "Mode set to " + payload->mode;
                std::cout << "[Controller] Mode changed to: " << payload->mode << std::endl;
            } else {
                ack.status = ipc::AckStatus::ERROR;
                ack.message = "Invalid mode value: " + payload->mode;
            }
        }
        
        std::string response = json::create_ack_message(ack);
        send_message(client_socket, response);
        
    } else if (msg.type == ipc::MessageType::SET_TARGETS) {
        auto payload = json::parse_set_targets(msg.payload_json);
        if (!payload.has_value()) {
            ack.status = ipc::AckStatus::ERROR;
            ack.message = "Invalid set_targets payload";
        } else {
            if (payload->target_abv.has_value()) {
                state.target_abv = payload->target_abv.value();
            }
            if (payload->target_flow.has_value()) {
                state.target_flow = payload->target_flow.value();
            }
            
            ack.status = ipc::AckStatus::OK;
            ack.message = "Targets updated";
            std::cout << "[Controller] Targets updated - ABV: " << state.target_abv 
                      << ", Flow: " << state.target_flow << std::endl;
        }
        
        std::string response = json::create_ack_message(ack);
        send_message(client_socket, response);
        
    } else {
        ack.status = ipc::AckStatus::ERROR;
        ack.message = "Unknown command type: " + msg.type;
        
        std::string response = json::create_ack_message(ack);
        send_message(client_socket, response);
    }
}

// Handle client connection
void handle_client(socket_t client_socket, ControllerState& state) {
    std::cout << "[Controller] Client connected" << std::endl;
    
    // Start telemetry thread (10 Hz)
    std::atomic<bool> telemetry_running{true};
    std::thread telemetry_thread([&]() {
        while (telemetry_running && state.running) {
            auto telemetry = state.get_telemetry();
            std::string message = json::create_telemetry_message(telemetry);
            
            if (!send_message(client_socket, message)) {
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 Hz
        }
    });
    
    // Buffer for incoming data
    char buffer[4096];
    std::string incomplete_message;
    
    while (state.running) {
#ifdef _WIN32
        int received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#else
        ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#endif
        
        if (received <= 0) {
            // Connection closed or error
            break;
        }
        
        buffer[received] = '\0';
        incomplete_message += buffer;
        
        // Process complete messages (newline-delimited)
        size_t newline_pos;
        while ((newline_pos = incomplete_message.find('\n')) != std::string::npos) {
            std::string message_str = incomplete_message.substr(0, newline_pos);
            incomplete_message = incomplete_message.substr(newline_pos + 1);
            
            // Parse and process message
            auto msg = json::parse_message(message_str);
            if (msg.has_value()) {
                process_command(client_socket, state, msg.value());
            } else {
                std::cerr << "[Controller] Failed to parse message: " << message_str << std::endl;
                // Invalid JSON - close connection per protocol
                telemetry_running = false;
                telemetry_thread.join();
                CLOSE_SOCKET(client_socket);
                std::cout << "[Controller] Client disconnected (invalid JSON)" << std::endl;
                return;
            }
        }
    }
    
    telemetry_running = false;
    telemetry_thread.join();
    CLOSE_SOCKET(client_socket);
    std::cout << "[Controller] Client disconnected" << std::endl;
}

int main() {
    std::cout << "[Controller] Starting Equilibria Controller (IPC v" 
              << ipc::PROTOCOL_VERSION << ")" << std::endl;
    
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Controller] WSAStartup failed" << std::endl;
        return 1;
    }
#endif
    
    ControllerState state;
    
    // Create socket
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET_VAL) {
        std::cerr << "[Controller] Failed to create socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Set socket options
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // Bind socket
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipc::DEFAULT_HOST);
    server_addr.sin_port = htons(ipc::DEFAULT_PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "[Controller] Failed to bind to " << ipc::DEFAULT_HOST 
                  << ":" << ipc::DEFAULT_PORT << std::endl;
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Listen for connections
    if (listen(server_socket, 5) == SOCKET_ERROR_VAL) {
        std::cerr << "[Controller] Failed to listen" << std::endl;
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    std::cout << "[Controller] Listening on " << ipc::DEFAULT_HOST 
              << ":" << ipc::DEFAULT_PORT << std::endl;
    
    // Accept connections (one client at a time for MVP)
    while (state.running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket == INVALID_SOCKET_VAL) {
            continue;
        }
        
        handle_client(client_socket, state);
    }
    
    CLOSE_SOCKET(server_socket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    std::cout << "[Controller] Shutdown complete" << std::endl;
    
    return 0;
}
