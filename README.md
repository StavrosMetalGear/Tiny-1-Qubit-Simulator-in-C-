# Tiny Quantum Simulator (C++)

A minimal quantum computing simulator written in modern C++.

This repository contains:
- an educational **1-qubit simulator** (interactive CLI)
- a scalable **N-qubit statevector simulator**
- optional **multithreaded** gate application (for performance experiments)

The goal is to build the simulator in clear phases: correctness first, then performance and usability.

---

## Project Structure

---

## Features

### 1-Qubit Simulator (CLI)
- State representation:  |ψ⟩ = α|0⟩ + β|1⟩
- Complex amplitudes via `std::complex<double>`
- Gates: X, Z, H
- Measurement with probabilistic collapse
- Interactive command loop

### N-Qubit Statevector Simulator
- Statevector size: 2ⁿ complex amplitudes
- Single-qubit gates on any target qubit (0 = least-significant bit)
- Measurement of an arbitrary qubit with collapse + renormalization
- Optional multithreading for gate application (`--threads T`)

---

## Build

### Requirements
- C++17 compatible compiler
- CMake (Visual Studio CMake integration works)

### Build (generic CMake)
```bash
cmake -S . -B build
cmake --build build

