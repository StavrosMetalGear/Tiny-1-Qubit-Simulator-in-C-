cmake_minimum_required(VERSION 3.16)
project(tiny_quantum_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(QuantumSimulator
    src/main.cpp
    src/qubit.cpp
    src/statevector.cpp
)

target_include_directories(QuantumSimulator PRIVATE ${CMAKE_SOURCE_DIR}/include)
