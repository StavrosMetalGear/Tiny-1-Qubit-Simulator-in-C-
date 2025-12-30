#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <chrono>
#include <cstdint>

#include "qubit.hpp"
#include "statevector.hpp"

static void print_help() {
    std::cout <<
        R"(Usage:
  CMakeTarget --mode qubit
  CMakeTarget --mode nqubit --qubits N --threads T
  CMakeTarget --mode bell --threads T --shots S
  QuantumSimulator --mode bench --qubits N --depth D --threads T [--seed S] [--compare]
  QuantumSimulator --mode bench --qubits N --depth D --threads T [--seed S] [--compare]
  QuantumSimulator --mode bell --threads T --shots S [--seed S]

Modes:
  ...
  bench   : benchmark random 1-qubit gates on an N-qubit statevector
  bench   : benchmark random 1-qubit gates on an N-qubit statevector
Examples:
  CMakeTarget --mode qubit
  CMakeTarget --mode nqubit --qubits 20 --threads 8

Modes:
  qubit   : interactive 1-qubit simulator
  nqubit  : small demo on N-qubit statevector (applies H then measures qubit 0)
  bell    : Bell state demo with 2 qubits (creates |Ö+> and measures correlations
  bell    : prepares Bell state (H + CNOT) and measures statistics
  QuantumSimulator --mode bell --threads T --shots S [--seed S]

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
static void run_bell_demo(std::size_t threads, std::size_t shots, std::uint64_t seed) {
    //std::mt19937_64 rng(std::random_device{}());
    std::mt19937_64 rng(seed);

    // Bell state on 2 qubits:
    // Start |00>
    // H on qubit 0
    // CNOT control=0 target=1
    // => (|00> + |11>) / sqrt(2)
    qsim::StateVector sv(2);

    sv.H(0, threads);
    sv.CNOT(0, 1, threads);

    std::cout << "Bell State Demo (|Ö+>)\n";
    std::cout << "----------------------\n";
    std::cout << "Threads: " << threads << "\n";
    std::cout << "Shots:   " << shots << "\n";
    std::cout << "Norm^2:  " << sv.norm2() << "\n\n";

    // Measure many times to show correlations.
    // IMPORTANT: measurement collapses state, so re-prepare each shot.
    std::size_t c00 = 0, c01 = 0, c10 = 0, c11 = 0;

    for (std::size_t s = 0; s < shots; ++s) {
        qsim::StateVector tmp(2);
        tmp.H(0, threads);
        tmp.CNOT(0, 1, threads);

        int b0 = tmp.measure_qubit(0, rng);
        int b1 = tmp.measure_qubit(1, rng);

        if (b0 == 0 && b1 == 0) ++c00;
        else if (b0 == 0 && b1 == 1) ++c01;
        else if (b0 == 1 && b1 == 0) ++c10;
        else ++c11;
    }

    std::cout << "Counts:\n";
    std::cout << "  00: " << c00 << "\n";
    std::cout << "  01: " << c01 << "\n";
    std::cout << "  10: " << c10 << "\n";
    std::cout << "  11: " << c11 << "\n";
    std::cout << "\nExpected: mostly 00 and 11 (about 50/50), near-zero 01 and 10.\n";
}
static void run_bench(std::uint32_t n,
    std::size_t depth,
    std::size_t threads,
    std::uint64_t seed,
    bool compare_single_thread)
{
    using clock = std::chrono::steady_clock;

    auto run_once = [&](std::size_t tcount) -> double {
        qsim::StateVector sv(n);
        std::mt19937_64 rng(seed);

        std::uniform_int_distribution<std::uint32_t> qb(0, n - 1);
        std::uniform_int_distribution<int> gate(0, 2); // 0=X,1=Z,2=H

        auto t0 = clock::now();
        for (std::size_t d = 0; d < depth; ++d) {
            std::uint32_t target = qb(rng);
            int g = gate(rng);

            if (g == 0) sv.X(target, tcount);
            else if (g == 1) sv.Z(target, tcount);
            else sv.H(target, tcount);
        }
        auto t1 = clock::now();

        std::chrono::duration<double, std::milli> ms = t1 - t0;
        return ms.count();
        };

    std::cout << "Benchmark Mode\n";
    std::cout << "--------------\n";
    std::cout << "Qubits:   " << n << "\n";
    std::cout << "Depth:    " << depth << " gates\n";
    std::cout << "Threads:  " << threads << "\n";
    std::cout << "Seed:     " << seed << "\n\n";

    if (compare_single_thread) {
        double ms1 = run_once(1);
        double msT = run_once(threads);

        std::cout << "Runtime (1 thread):   " << ms1 << " ms\n";
        std::cout << "Runtime (" << threads << " threads): " << msT << " ms\n";
        if (msT > 0.0) {
            std::cout << "Speedup: " << (ms1 / msT) << "x\n";
        }
    }
    else {
        double ms = run_once(threads);
        std::cout << "Runtime: " << ms << " ms\n";
    }

    std::cout << "\nTip: try bigger n (e.g., 18–24) and larger depth to see speedups.\n";
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
    if (mode == "bell") {
        std::size_t threads = static_cast<std::size_t>(std::stoul(get_arg(args, "--threads", "1")));
        std::size_t shots = static_cast<std::size_t>(std::stoul(get_arg(args, "--shots", "1000")));
        std::uint64_t seed = static_cast<std::uint64_t>(std::stoull(get_arg(args, "--seed", "123")));

        run_bell_demo(threads, shots);
        return 0;
    }
    if (mode == "bench") {
        std::uint32_t n = static_cast<std::uint32_t>(std::stoul(get_arg(args, "--qubits", "20")));
        std::size_t depth = static_cast<std::size_t>(std::stoul(get_arg(args, "--depth", "200")));
        std::size_t threads = static_cast<std::size_t>(std::stoul(get_arg(args, "--threads", "1")));
        std::uint64_t seed = static_cast<std::uint64_t>(std::stoull(get_arg(args, "--seed", "123")));

        bool compare = has_flag(args, "--compare");
        if (threads < 1) threads = 1;

        run_bench(n, depth, threads, seed, compare);
        return 0;
    }

    std::cout << "Unknown mode: " << mode << "\n";
    print_help();
    return 1;
}
