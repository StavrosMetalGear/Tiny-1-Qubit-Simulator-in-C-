#pragma once

#include <QMainWindow>

class QComboBox;
class QSpinBox;
class QPushButton;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onRunBell();
    void onRunBench();
    void onApplyH0();

private:
    void logLine(const QString& s);

    QComboBox* mode_;
    QSpinBox* qubits_;
    QSpinBox* threads_;
    QSpinBox* shots_;
    QSpinBox* depth_;
    QSpinBox* seed_;
    

    QTextEdit* output_;
    QPushButton* bellBtn_;
    QPushButton* benchBtn_;
    QPushButton* hBtn_;
};
#pragma once
