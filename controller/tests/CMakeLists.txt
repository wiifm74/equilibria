cmake_minimum_required(VERSION 3.15)
project(equilibria_controller_tests VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find nlohmann_json
find_package(nlohmann_json 3.11.0 REQUIRED)

# Test executable
add_executable(test_ipc_protocol
    test_ipc_protocol.cpp
    ../src/ipc_protocol.cpp
)

target_include_directories(test_ipc_protocol PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(test_ipc_protocol PRIVATE
    nlohmann_json::nlohmann_json
)

# Enable testing
enable_testing()

# Add test to CTest
add_test(NAME ipc_protocol_tests COMMAND test_ipc_protocol)

# Set test properties for CI
set_tests_properties(ipc_protocol_tests PROPERTIES
    TIMEOUT 10
    LABELS "unit;fast"
)
