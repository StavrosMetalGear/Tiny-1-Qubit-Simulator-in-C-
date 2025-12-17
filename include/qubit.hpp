#pragma once
#include <complex>
#include <array>
#include <random>
#include <string>

#include "gates.hpp"

namespace qsim {

    class Qubit {
    public:
        Qubit(Complex alpha = { 1.0, 0.0 }, Complex beta = { 0.0, 0.0 });

        static Qubit basis0();
        static Qubit basis1();

        void apply_gate(const Gate2x2& U);

        void X();
        void Z();
        void H();

        int measure(std::mt19937_64& rng);

        void print_state() const;

    private:
        Complex alpha_;
        Complex beta_;

        void normalize();
        static std::string format_complex(const Complex& z);
    };

} // namespace qsim

