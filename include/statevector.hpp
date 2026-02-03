#pragma once
#include <vector>
#include <complex>
#include <cstdint>
#include <random>
#include <stdexcept>

#include "gates.hpp"

namespace qsim {
    

    class StateVector {
    public:
        explicit StateVector(std::uint32_t num_qubits);

        std::uint32_t num_qubits() const { return n_; }
        std::size_t dim() const { return state_.size(); }

        // Read-only access for observers/debug
        const std::vector<Complex>& amplitudes() const { return state_; }

        void set_zero_state(); // |00..0>

        // Apply 1-qubit gates to target qubit (0 = least significant bit)
        void apply_gate_1q(std::uint32_t target, const Gate2x2& U, std::size_t threads = 1);
        void X(std::uint32_t target, std::size_t threads = 1);
        void Z(std::uint32_t target, std::size_t threads = 1);
        void H(std::uint32_t target, std::size_t threads = 1);

        // Measure one qubit (returns 0/1), collapses state
        int measure_qubit(std::uint32_t target, std::mt19937_64& rng);

        // Quick sanity check: sum(|amp|^2)
        double norm2() const;
        // Two-qubit gate
        void CNOT(std::uint32_t control, std::uint32_t target, std::size_t threads = 1);
    private:
        std::uint32_t n_;
        std::vector<Complex> state_;

        void apply_gate_1q_st(std::uint32_t target, const Gate2x2& U);
        void apply_gate_1q_mt(std::uint32_t target, const Gate2x2& U, std::size_t threads);

        void renormalize_inplace();
    };

} // namespace qsim
