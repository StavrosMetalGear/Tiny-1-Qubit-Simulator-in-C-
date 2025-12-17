#include <iostream>
#include <string>
#include <random>
#include <vector>

#include "qubit.hpp"
#include "statevector.hpp"

static void print_help() {
    std::cout <<
        R"(Usage:
  CMakeTarget --mode qubit
  CMakeTarget --mode nqubit --qubits N --threads T

Examples:
  CMakeTarget --mode qubit
  CMakeTarget --mode nqubit --qubits 20 --threads 8

Modes:
  qubit   : interactive 1-qubit simulator
  nqubit  : small demo on N-qubit statevector (applies H then measures qubit 0)
)";
}

static std::string get_arg(const std::vector<std::string>& args, const std::string& key, const std::string& def) {
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key) return args[i + 1];
    }
    return def;
}

static bool has_flag(const std::vector<std::string>& args, const std::string& key) {
    for (auto& a : args) if (a == key) return true;
    return false;
}

static void run_qubit_mode() {
    std::mt19937_64 rng(std::random_device{}());

    std::cout << "Tiny 1-Qubit Simulator\n";
    std::cout << "----------------------\n";

    std::cout << "Choose initial state:\n";
    std::cout << "  0: |0>\n";
    std::cout << "  1: |1>\n";
    std::cout << "  c: custom a, b (normalized automatically)\n";
    std::cout << "Your choice (0/1/c): ";

    char choice;
    std::cin >> choice;

    qsim::Qubit qubit;

    if (choice == '0') qubit = qsim::Qubit::basis0();
    else if (choice == '1') qubit = qsim::Qubit::basis1();
    else if (choice == 'c' || choice == 'C') {
        double a_re, a_im, b_re, b_im;
        std::cout << "Enter a (real imag): ";
        std::cin >> a_re >> a_im;
        std::cout << "Enter b (real imag): ";
        std::cin >> b_re >> b_im;
        qubit = qsim::Qubit({ a_re, a_im }, { b_re, b_im });
    }
    else {
        std::cout << "Invalid choice, defaulting to |0>.\n";
        qubit = qsim::Qubit::basis0();
    }

    qubit.print_state();

    std::cout << "Commands:\n"
        << "  x        Apply Pauli-X gate\n"
        << "  z        Apply Pauli-Z gate\n"
        << "  h        Apply Hadamard gate\n"
        << "  m        Measure in computational basis\n"
        << "  s        Show current state\n"
        << "  q        Quit\n";

    while (true) {
        std::cout << "\n> ";
        char cmd;
        std::cin >> cmd;
        if (!std::cin) break;

        if (cmd == 'q') break;

        switch (cmd) {
        case 'x': qubit.X(); std::cout << "Applied X gate.\n"; break;
        case 'z': qubit.Z(); std::cout << "Applied Z gate.\n"; break;
        case 'h': qubit.H(); std::cout << "Applied H gate.\n"; break;
        case 'm': {
            int r = qubit.measure(rng);
            std::cout << "Measurement result: " << r << "\n";
            break;
        }
        case 's': qubit.print_state(); break;
        default: std::cout << "Unknown command.\n"; break;
        }
    }
}

static void run_nqubit_demo(std::uint32_t n, std::size_t threads) {
    std::mt19937_64 rng(std::random_device{}());

    qsim::StateVector sv(n);

    std::cout << "N-Qubit StateVector Demo\n";
    std::cout << "------------------------\n";
    std::cout << "Qubits: " << n << "\n";
    std::cout << "Threads: " << threads << "\n";

    // Demo circuit: H on qubit 0 then measure qubit 0
    sv.H(0, threads);
    std::cout << "After H on qubit 0, norm^2 = " << sv.norm2() << "\n";

    int outcome = sv.measure_qubit(0, rng);
    std::cout << "Measured qubit 0 -> " << outcome << "\n";
    std::cout << "After collapse, norm^2 = " << sv.norm2() << "\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (has_flag(args, "--help") || has_flag(args, "-h")) {
        print_help();
        return 0;
    }

    std::string mode = get_arg(args, "--mode", "qubit");

    if (mode == "qubit") {
        run_qubit_mode();
        return 0;
    }

    if (mode == "nqubit") {
        std::uint32_t n = static_cast<std::uint32_t>(std::stoul(get_arg(args, "--qubits", "4")));
        std::size_t threads = static_cast<std::size_t>(std::stoul(get_arg(args, "--threads", "1")));
        run_nqubit_demo(n, threads);
        return 0;
    }

    std::cout << "Unknown mode: " << mode << "\n";
    print_help();
    return 1;
}
