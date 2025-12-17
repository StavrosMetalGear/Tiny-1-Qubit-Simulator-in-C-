#include "qubit.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace qsim {

    Qubit::Qubit(Complex alpha, Complex beta) : alpha_(alpha), beta_(beta) {
        normalize();
    }

    Qubit Qubit::basis0() { return Qubit({ 1.0, 0.0 }, { 0.0, 0.0 }); }
    Qubit Qubit::basis1() { return Qubit({ 0.0, 0.0 }, { 1.0, 0.0 }); }

    void Qubit::normalize() {
        double norm_sq = std::norm(alpha_) + std::norm(beta_);
        if (norm_sq == 0.0) {
            alpha_ = Complex(1.0, 0.0);
            beta_ = Complex(0.0, 0.0);
            return;
        }
        double inv_norm = 1.0 / std::sqrt(norm_sq);
        alpha_ *= inv_norm;
        beta_ *= inv_norm;
    }

    void Qubit::apply_gate(const Gate2x2& U) {
        Complex a_new = U[0][0] * alpha_ + U[0][1] * beta_;
        Complex b_new = U[1][0] * alpha_ + U[1][1] * beta_;
        alpha_ = a_new;
        beta_ = b_new;
        normalize();
    }

    void Qubit::X() { apply_gate(X_gate()); }
    void Qubit::Z() { apply_gate(Z_gate()); }
    void Qubit::H() { apply_gate(H_gate()); }

    int Qubit::measure(std::mt19937_64& rng) {
        double p0 = std::norm(alpha_);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng);

        if (r < p0) {
            alpha_ = Complex(1.0, 0.0);
            beta_ = Complex(0.0, 0.0);
            return 0;
        }
        else {
            alpha_ = Complex(0.0, 0.0);
            beta_ = Complex(1.0, 0.0);
            return 1;
        }
    }

    std::string Qubit::format_complex(const Complex& z) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4) << z.real();
        if (z.imag() >= 0) oss << " + " << z.imag() << "i";
        else              oss << " - " << -z.imag() << "i";
        return oss.str();
    }

    void Qubit::print_state() const {
        std::cout << "State |psi> = a|0> + b|1>\n";
        std::cout << "  a = " << format_complex(alpha_) << "  (|a|^2 = " << std::norm(alpha_) << ")\n";
        std::cout << "  b = " << format_complex(beta_) << "  (|b|^2 = " << std::norm(beta_) << ")\n";
        std::cout << "  Check: |a|^2 + |b|^2 = " << (std::norm(alpha_) + std::norm(beta_)) << "\n";
    }

} // namespace qsim
