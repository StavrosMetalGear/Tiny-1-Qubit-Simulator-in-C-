cmake_minimum_required(VERSION 3.16)
project(QuantumSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library (reusable simulator engine)
add_library(qsim_core
    src/qubit.cpp
    src/statevector.cpp
)

target_include_directories(qsim_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# CLI executable (depends on core)
add_executable(QuantumSimulator
    src/main.cpp
)

target_link_libraries(QuantumSimulator PRIVATE qsim_core)

