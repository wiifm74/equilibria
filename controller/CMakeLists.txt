cmake_minimum_required(VERSION 3.20)
project(equilibria_controller LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files for the controller executable
add_executable(equilibria-controller
  src/main.cpp
  src/ipc_server.cpp
  src/ipc_protocol.cpp
  src/controller.cpp
)

# Include directories
# Allow #include "ipc_server.hpp" and other headers to resolve from src/
# Also include third_party for minimal_json
target_include_directories(equilibria-controller PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Platform-specific libraries
if(WIN32)
  # Link Winsock2 for TCP/IP networking on Windows
  target_link_libraries(equilibria-controller PRIVATE ws2_32)
endif()

# Compiler warnings
if(MSVC)
  target_compile_options(equilibria-controller PRIVATE /W4)
else()
  target_compile_options(equilibria-controller PRIVATE -Wall -Wextra -pedantic)
endif()
