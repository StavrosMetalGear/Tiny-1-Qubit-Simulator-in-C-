#include "MainWindow.hpp"

#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QThread>

#include <random>
#include <chrono>

#include "qubit.hpp"
#include "statevector.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    mode_ = new QComboBox(this);
    mode_->addItem("1-Qubit");
    mode_->addItem("N-Qubit");

    qubits_ = new QSpinBox(this);
    qubits_->setRange(1, 30);
    qubits_->setValue(4);

    threads_ = new QSpinBox(this);
    int maxThreads = (int)QThread::idealThreadCount();
    if (maxThreads < 1) maxThreads = 1;
    threads_->setRange(1, std::max(1, maxThreads));
    threads_->setValue(std::min(4, threads_->maximum()));
    seed_ = new QSpinBox(this);
    seed_->setRange(0, 2000000000);
    seed_->setValue(123);

    shots_ = new QSpinBox(this);
    shots_->setRange(1, 200000);
    shots_->setValue(2000);

    depth_ = new QSpinBox(this);
    depth_->setRange(1, 200000);
    depth_->setValue(2000);

    bellBtn_ = new QPushButton("Run Bell Demo", this);
    benchBtn_ = new QPushButton("Run Bench", this);
    hBtn_ = new QPushButton("Apply H on qubit 0", this);

    output_ = new QTextEdit(this);
    output_->setReadOnly(true);

    auto* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Mode:"));
    topRow->addWidget(mode_);
    topRow->addSpacing(12);
    topRow->addWidget(new QLabel("Qubits:"));
    topRow->addWidget(qubits_);
    topRow->addSpacing(12);
    topRow->addWidget(new QLabel("Threads:"));
    topRow->addWidget(threads_);

    auto* row2 = new QHBoxLayout();
    row2->addWidget(new QLabel("Shots:"));
    row2->addWidget(shots_);
    row2->addSpacing(12);
    row2->addWidget(new QLabel("Depth:"));
    row2->addWidget(depth_);
    row2->addSpacing(12);
    row2->addWidget(new QLabel("Seed:"));
    row2->addWidget(seed_);


    auto* buttons = new QHBoxLayout();
    buttons->addWidget(hBtn_);
    buttons->addWidget(bellBtn_);
    buttons->addWidget(benchBtn_);

    auto* layout = new QVBoxLayout();
    layout->addLayout(topRow);
    layout->addLayout(row2);
    layout->addLayout(buttons);
    layout->addWidget(output_);

    central->setLayout(layout);

    setWindowTitle("QuantumSimulator GUI (Qt)");

    connect(bellBtn_, &QPushButton::clicked, this, &MainWindow::onRunBell);
    connect(benchBtn_, &QPushButton::clicked, this, &MainWindow::onRunBench);
    connect(hBtn_, &QPushButton::clicked, this, &MainWindow::onApplyH0);

    logLine("Ready. Choose mode, qubits, threads.");
    logLine("Tip: Bench with qubits ~20-26 and depth > 1000 for noticeable speedups.");
}

void MainWindow::logLine(const QString& s) {
    output_->append(s);
}

void MainWindow::onApplyH0() {
    const std::size_t T = (std::size_t)threads_->value();
    const int modeIndex = mode_->currentIndex();

    if (modeIndex == 0) {
        // 1-qubit: show probabilities after H
        qsim::Qubit q = qsim::Qubit::basis0();
        q.H();
        logLine("1-Qubit: prepared |0>, applied H. (Use CLI for interactive gates)");
        // We don't have a "to string" yet; just indicate it worked.
    }
    else {
        const std::uint32_t n = (std::uint32_t)qubits_->value();
        qsim::StateVector sv(n);
        sv.H(0, T);
        logLine(QString("N-Qubit: applied H on qubit 0 (n=%1, threads=%2). norm^2=%3")
            .arg(n).arg((int)T).arg(sv.norm2(), 0, 'g', 12));
    }
}

void MainWindow::onRunBell() {
    const std::size_t T = (std::size_t)threads_->value();
    const std::size_t shots = (std::size_t)shots_->value();
    std::mt19937_64 rng((std::uint64_t)seed_->value());


    std::size_t c00 = 0, c01 = 0, c10 = 0, c11 = 0;

    for (std::size_t s = 0; s < shots; ++s) {
        qsim::StateVector tmp(2);
        tmp.H(0, T);
        tmp.CNOT(0, 1, T);

        int b0 = tmp.measure_qubit(0, rng);
        int b1 = tmp.measure_qubit(1, rng);

        if (b0 == 0 && b1 == 0) ++c00;
        else if (b0 == 0 && b1 == 1) ++c01;
        else if (b0 == 1 && b1 == 0) ++c10;
        else ++c11;
    }

    logLine(QString("Bell demo (threads=%1, shots=%2): 00=%3  01=%4  10=%5  11=%6")
        .arg((int)T).arg((int)shots).arg((int)c00).arg((int)c01).arg((int)c10).arg((int)c11));
    logLine("Expected: mostly 00 and 11 (about 50/50).");
}

void MainWindow::onRunBench() {
    using clock = std::chrono::steady_clock;

    const std::uint32_t n = (std::uint32_t)qubits_->value();
    const std::size_t depth = (std::size_t)depth_->value();
    const std::size_t T = (std::size_t)threads_->value();

    auto run_once = [&](std::size_t threads) -> double {
        qsim::StateVector sv(n);
        std::mt19937_64 rng((std::uint64_t)seed_->value());


        std::uniform_int_distribution<std::uint32_t> qb(0, n - 1);
        std::uniform_int_distribution<int> gate(0, 2);

        auto t0 = clock::now();
        for (std::size_t d = 0; d < depth; ++d) {
            std::uint32_t target = qb(rng);
            int g = gate(rng);
            if (g == 0) sv.X(target, threads);
            else if (g == 1) sv.Z(target, threads);
            else sv.H(target, threads);
        }
        auto t1 = clock::now();
        std::chrono::duration<double, std::milli> ms = t1 - t0;
        return ms.count();
        };

    // Compare 1 vs T
    double ms1 = run_once(1);
    double msT = run_once(T);
    double speedup = (msT > 0.0) ? (ms1 / msT) : 0.0;

    logLine(QString("Bench n=%1 depth=%2 : 1 thread=%3 ms, %4 threads=%5 ms, speedup=%6x")
        .arg(n).arg((int)depth)
        .arg(ms1, 0, 'f', 2)
        .arg((int)T)
        .arg(msT, 0, 'f', 2)
        .arg(speedup, 0, 'f', 2));
}
