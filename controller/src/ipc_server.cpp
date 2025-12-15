#include "ipc_server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
namespace {
    struct WinsockInitializer {
        WinsockInitializer() {
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0) {
                std::cerr << "[IpcServer] WSAStartup failed: " << result << std::endl;
            } else {
                std::cout << "[IpcServer] Winsock initialized" << std::endl;
            }
        }
        ~WinsockInitializer() {
            WSACleanup();
            std::cout << "[IpcServer] Winsock cleaned up" << std::endl;
        }
    };
    // Global initializer for Winsock
    WinsockInitializer g_winsockInit;
}
#endif

namespace equilibria {

IpcServer::IpcServer(const std::string& host, int port)
    : host_(host)
    , port_(port)
    , serverSocket_(INVALID_SOCKET_VALUE)
    , running_(false)
{
}

IpcServer::~IpcServer() {
    stop();
}

bool IpcServer::start(MessageCallback callback) {
    if (running_.load()) {
        std::cerr << "[IpcServer] Server already running" << std::endl;
        return false;
    }

    if (!callback) {
        std::cerr << "[IpcServer] Message callback is required" << std::endl;
        return false;
    }

    messageCallback_ = callback;

    if (!initializeSocket()) {
        return false;
    }

    running_.store(true);
    serverThread_ = std::thread(&IpcServer::serverLoop, this);

    std::cout << "[IpcServer] Server started on " << host_ << ":" << port_ << std::endl;
    return true;
}

void IpcServer::stop() {
    if (!running_.load()) {
        return;
    }

    std::cout << "[IpcServer] Stopping server..." << std::endl;
    running_.store(false);

    // Close server socket to unblock accept()
    if (serverSocket_ != INVALID_SOCKET_VALUE) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET_VALUE;
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }

    cleanup();
    std::cout << "[IpcServer] Server stopped" << std::endl;
}

void IpcServer::send(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex_);

    std::string msg = message;
    if (msg.empty() || msg.back() != '\n') {
        msg += '\n';
    }

    std::vector<socket_t> deadClients;

    for (auto clientSocket : clients_) {
        int result = ::send(clientSocket, msg.c_str(), static_cast<int>(msg.size()), 0);
        if (result == SOCKET_ERROR_VALUE) {
            std::cerr << "[IpcServer] Failed to send to client, marking for removal" << std::endl;
            deadClients.push_back(clientSocket);
        }
    }

    // Remove dead clients
    for (auto deadSocket : deadClients) {
        closesocket(deadSocket);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), deadSocket), clients_.end());
    }

    if (!clients_.empty()) {
        std::cout << "[IpcServer] Sent message to " << clients_.size() << " client(s)" << std::endl;
    }
}

bool IpcServer::initializeSocket() {
    serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket_ == INVALID_SOCKET_VALUE) {
        std::cerr << "[IpcServer] Failed to create socket" << std::endl;
        return false;
    }

    // Set socket options
    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));
#else
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port_));
    inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr);

    if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        std::cerr << "[IpcServer] Failed to bind to " << host_ << ":" << port_ << std::endl;
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET_VALUE;
        return false;
    }

    if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR_VALUE) {
        std::cerr << "[IpcServer] Failed to listen on socket" << std::endl;
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET_VALUE;
        return false;
    }

    std::cout << "[IpcServer] Socket initialized and listening" << std::endl;
    return true;
}

void IpcServer::serverLoop() {
    std::cout << "[IpcServer] Server loop started" << std::endl;

    while (running_.load()) {
        sockaddr_in clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        socket_t clientSocket = accept(serverSocket_, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);

        if (clientSocket == INVALID_SOCKET_VALUE) {
            if (running_.load()) {
                std::cerr << "[IpcServer] Accept failed" << std::endl;
            }
            break;
        }

        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        std::cout << "[IpcServer] Client connected from " << clientIp << ":" << ntohs(clientAddr.sin_port) << std::endl;

        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            clients_.push_back(clientSocket);
        }

        // Handle client in the same thread (simple implementation)
        // For production, consider a thread pool or non-blocking I/O
        std::thread clientThread(&IpcServer::handleClient, this, clientSocket);
        clientThread.detach();
    }

    std::cout << "[IpcServer] Server loop ended" << std::endl;
}

void IpcServer::handleClient(socket_t clientSocket) {
    std::cout << "[IpcServer] Handling client connection" << std::endl;

    char buffer[4096];
    std::string lineBuffer;

    while (running_.load()) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            lineBuffer += buffer;

            // Process complete lines (newline-delimited messages)
            size_t pos;
            while ((pos = lineBuffer.find('\n')) != std::string::npos) {
                std::string message = lineBuffer.substr(0, pos);
                lineBuffer.erase(0, pos + 1);

                if (!message.empty()) {
                    // Remove carriage return if present (handle \r\n)
                    if (message.back() == '\r') {
                        message.pop_back();
                    }

                    if (!message.empty()) {
                        std::cout << "[IpcServer] Received message: " << message.substr(0, 80) 
                                  << (message.size() > 80 ? "..." : "") << std::endl;

                        // Call the callback with the message
                        if (messageCallback_) {
                            messageCallback_(message);
                        }
                    }
                }
            }
        } else if (bytesReceived == 0) {
            std::cout << "[IpcServer] Client disconnected" << std::endl;
            break;
        } else {
            std::cerr << "[IpcServer] Receive error" << std::endl;
            break;
        }
    }

    // Remove client from list
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), clientSocket), clients_.end());
    }

    closesocket(clientSocket);
    std::cout << "[IpcServer] Client handler finished" << std::endl;
}

void IpcServer::cleanup() {
    std::lock_guard<std::mutex> lock(clientsMutex_);

    for (auto clientSocket : clients_) {
        closesocket(clientSocket);
    }
    clients_.clear();

    std::cout << "[IpcServer] Cleanup completed" << std::endl;
}

} // namespace equilibria
