cmake_minimum_required(VERSION 3.10)
project(equilibria-controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include shared headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)

# Controller executable
add_executable(equilibria-controller
    main.cpp
)

# Link libraries
if(WIN32)
    target_link_libraries(equilibria-controller ws2_32)
else()
    target_link_libraries(equilibria-controller pthread)
endif()

# Set output directory
set_target_properties(equilibria-controller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
