cmake_minimum_required(VERSION 3.10)
project(equilibria-shared-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include shared headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Test executable
add_executable(test_ipc_protocol
    test_ipc_protocol.cpp
)

# Set output directory
set_target_properties(test_ipc_protocol PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
