#pragma once
#include <array>
#include <complex>
#include <cmath>

namespace qsim {

    using Complex = std::complex<double>;
    using Gate2x2 = std::array<std::array<Complex, 2>, 2>;

    inline Gate2x2 X_gate() {
        return { {
            {Complex(0,0), Complex(1,0)},
            {Complex(1,0), Complex(0,0)}
        } };
    }

    inline Gate2x2 Z_gate() {
        return { {
            {Complex(1,0), Complex(0,0)},
            {Complex(0,0), Complex(-1,0)}
        } };
    }

    inline Gate2x2 H_gate() {
        const double inv = 1.0 / std::sqrt(2.0);
        return { {
            {Complex(inv,0),  Complex(inv,0)},
            {Complex(inv,0),  Complex(-inv,0)}
        } };
    }

} // namespace qsim

