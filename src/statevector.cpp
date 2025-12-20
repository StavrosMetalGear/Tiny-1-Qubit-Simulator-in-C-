#include "statevector.hpp"
#include <thread>
#include <algorithm>
#include <cmath>

namespace qsim {
    static inline bool is_power_of_two(std::size_t x) {
        return x && ((x & (x - 1)) == 0);
    }

    StateVector::StateVector(std::uint32_t num_qubits) : n_(num_qubits) {
        if (n_ == 0) throw std::invalid_argument("num_qubits must be >= 1");
        state_.assign(std::size_t(1) << n_, Complex(0.0, 0.0));
        set_zero_state();
    }

    void StateVector::set_zero_state() {
        std::fill(state_.begin(), state_.end(), Complex(0.0, 0.0));
        state_[0] = Complex(1.0, 0.0);
    }

    double StateVector::norm2() const {
        double s = 0.0;
        for (const auto& a : state_) s += std::norm(a);
        return s;
    }

    void StateVector::renormalize_inplace() {
        double n2 = norm2();
        if (n2 == 0.0) {
            set_zero_state();
            return;
        }
        double inv = 1.0 / std::sqrt(n2);
        for (auto& a : state_) a *= inv;
    }

    void StateVector::apply_gate_1q(std::uint32_t target, const Gate2x2& U, std::size_t threads) {
        if (target >= n_) throw std::out_of_range("target qubit out of range");
        if (threads <= 1) apply_gate_1q_st(target, U);
        else              apply_gate_1q_mt(target, U, threads);
    }

    void StateVector::X(std::uint32_t target, std::size_t threads) { apply_gate_1q(target, X_gate(), threads); }
    void StateVector::Z(std::uint32_t target, std::size_t threads) { apply_gate_1q(target, Z_gate(), threads); }
    void StateVector::H(std::uint32_t target, std::size_t threads) { apply_gate_1q(target, H_gate(), threads); }

    void StateVector::apply_gate_1q_st(std::uint32_t target, const Gate2x2& U) {
        const std::size_t N = state_.size();
        const std::size_t mask = (std::size_t(1) << target);

        for (std::size_t i = 0; i < N; ++i) {
            if ((i & mask) == 0) {
                std::size_t j = i | mask;
                Complex a = state_[i];
                Complex b = state_[j];
                state_[i] = U[0][0] * a + U[0][1] * b;
                state_[j] = U[1][0] * a + U[1][1] * b;
            }
        }
    }

    void StateVector::apply_gate_1q_mt(std::uint32_t target, const Gate2x2& U, std::size_t threads) {
        const std::size_t N = state_.size();
        const std::size_t mask = (std::size_t(1) << target);
        const std::size_t half_pairs = N / 2;

        if (half_pairs == 0) return;
        threads = std::max<std::size_t>(1, threads);
        threads = std::min<std::size_t>(threads, half_pairs);

        auto worker = [&](std::size_t pair_begin, std::size_t pair_end) {
            for (std::size_t pair_id = pair_begin; pair_id < pair_end; ++pair_id) {
                // Map pair_id -> index0 where target bit is 0
                std::size_t low = pair_id & (mask - 1);
                std::size_t high = pair_id & ~(mask - 1);
                std::size_t i = (high << 1) | low; // target bit = 0
                std::size_t j = i | mask;          // target bit = 1

                Complex a = state_[i];
                Complex b = state_[j];
                state_[i] = U[0][0] * a + U[0][1] * b;
                state_[j] = U[1][0] * a + U[1][1] * b;
            }
            };

        std::vector<std::thread> pool;
        pool.reserve(threads);

        std::size_t chunk = half_pairs / threads;
        std::size_t rem = half_pairs % threads;

        std::size_t start = 0;
        for (std::size_t t = 0; t < threads; ++t) {
            std::size_t size = chunk + (t < rem ? 1 : 0);
            std::size_t end = start + size;
            pool.emplace_back(worker, start, end);
            start = end;
        }
        for (auto& th : pool) th.join();
    }

    int StateVector::measure_qubit(std::uint32_t target, std::mt19937_64& rng) {
        if (target >= n_) throw std::out_of_range("target qubit out of range");

        const std::size_t N = state_.size();
        const std::size_t mask = (std::size_t(1) << target);

        double p0 = 0.0;
        for (std::size_t i = 0; i < N; ++i) {
            if ((i & mask) == 0) p0 += std::norm(state_[i]);
        }

        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng);
        int outcome = (r < p0) ? 0 : 1;

        // Collapse: zero incompatible amplitudes
        for (std::size_t i = 0; i < N; ++i) {
            if (((i & mask) == 0 && outcome == 1) || ((i & mask) != 0 && outcome == 0)) {
                state_[i] = Complex(0.0, 0.0);
            }
        }
        renormalize_inplace();
        return outcome;
    }
    void StateVector::CNOT(std::uint32_t control, std::uint32_t target, std::size_t threads) {
        if (control >= n_ || target >= n_) throw std::out_of_range("CNOT qubit index out of range");
        if (control == target) throw std::invalid_argument("CNOT control and target must be different");

        const std::size_t N = state_.size();
        if (!is_power_of_two(N)) throw std::runtime_error("State vector size must be power of two");

        const std::size_t cMask = (std::size_t(1) << control);
        const std::size_t tMask = (std::size_t(1) << target);

        // CNOT: if control bit is 1, flip target bit.
        // Implementation: swap amplitudes of |...1...0...> and |...1...1...> for the target bit,
        // but only for basis indices with control=1 and target=0 (so we swap each pair once).

        if (threads <= 1) {
            for (std::size_t i = 0; i < N; ++i) {
                if ((i & cMask) && ((i & tMask) == 0)) {
                    std::size_t j = i | tMask; // flip target to 1
                    std::swap(state_[i], state_[j]);
                }
            }
            return;
        }

        // Multithreaded: split the index range into chunks.
        threads = std::max<std::size_t>(1, threads);
        threads = std::min<std::size_t>(threads, N);

        auto worker = [&](std::size_t begin, std::size_t end) {
            for (std::size_t i = begin; i < end; ++i) {
                if ((i & cMask) && ((i & tMask) == 0)) {
                    std::size_t j = i | tMask;
                    std::swap(state_[i], state_[j]);
                }
            }
            };

        std::vector<std::thread> pool;
        pool.reserve(threads);

        std::size_t chunk = N / threads;
        std::size_t rem = N % threads;

        std::size_t start = 0;
        for (std::size_t t = 0; t < threads; ++t) {
            std::size_t size = chunk + (t < rem ? 1 : 0);
            std::size_t end = start + size;
            pool.emplace_back(worker, start, end);
            start = end;
        }
        for (auto& th : pool) th.join();
    }


} // namespace qsim
