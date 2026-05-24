#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QTimer>
#include <QString>
#include <QFile>

class TouchInput;
class WifiDialog;
class TerminalDialog;
class EthDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openWifiSettings();
    void openEthSettings();
    void onReboot();
    void updateTime();
    void clearLog();
    void cleanDiskSpace();
    void updateCode();
    void viewLogs();
    void updateSystemInfo();

private:
    void writeLog(const QString &message);
    double getCPUUsage();
    double getMemoryUsage();
    double getDiskUsage();

    QLabel *timeLabel;
    QLabel *cpuLabel;
    QLabel *memoryLabel;
    QLabel *diskLabel;
    QPushButton *wifiButton;
    QPushButton *ethButton;
    WifiDialog *wifiDialog;
    EthDialog *ethDialog;
    QTimer *timeTimer;
    QTimer *sysInfoTimer;
    QFile logFile;
};

#endif // MAINWINDOW_H
