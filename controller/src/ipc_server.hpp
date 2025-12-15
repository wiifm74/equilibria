#pragma once

#include <functional>
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
#endif

namespace equilibria {

/**
 * @brief TCP server for controller IPC on 127.0.0.1:7002
 * 
 * Uses newline-delimited UTF-8 JSON messages (one JSON object per line).
 * Single-threaded with a background thread for accept+read.
 * Avoids dynamic allocations in the control loop by using a callback mechanism.
 */
class IpcServer {
public:
    using MessageCallback = std::function<void(const std::string& message)>;

    IpcServer(const std::string& host = "127.0.0.1", int port = 7002);
    ~IpcServer();

    // Non-copyable, non-movable
    IpcServer(const IpcServer&) = delete;
    IpcServer& operator=(const IpcServer&) = delete;

    /**
     * @brief Start the TCP server and begin accepting connections
     * @param callback Function to call when a complete message is received
     * @return true on success, false on failure
     */
    bool start(MessageCallback callback);

    /**
     * @brief Stop the server and close all connections
     */
    void stop();

    /**
     * @brief Send a message to all connected clients
     * @param message Newline-delimited JSON message (newline will be added if missing)
     */
    void send(const std::string& message);

    /**
     * @brief Check if server is running
     */
    bool isRunning() const { return running_.load(); }

private:
    void serverLoop();
    void handleClient(socket_t clientSocket);
    bool initializeSocket();
    void cleanup();

    std::string host_;
    int port_;
    socket_t serverSocket_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    MessageCallback messageCallback_;

    // Connected clients
    std::vector<socket_t> clients_;
    std::mutex clientsMutex_;
};

} // namespace equilibria
