cmake_minimum_required(VERSION 3.16)
project(QuantumSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------
# Core library
# -----------------------
add_library(qsim_core
    src/qubit.cpp
    src/statevector.cpp
)

target_include_directories(qsim_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# -----------------------
# CLI app
# -----------------------
add_executable(QuantumSimulator
    src/main.cpp
)
target_link_libraries(QuantumSimulator PRIVATE qsim_core)

# -----------------------
# Qt GUI app (Widgets)
# -----------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(QuantumSimulatorGUI
    gui/main_gui.cpp
    gui/MainWindow.cpp
    gui/MainWindow.hpp
)

target_link_libraries(QuantumSimulatorGUI
    PRIVATE
        Qt6::Widgets
        qsim_core
)

target_include_directories(QuantumSimulatorGUI
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/gui
)
