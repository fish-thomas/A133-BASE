#include "mainwindow.h"
#include "wifidialog.h"
#include "terminaldialog.h"
#include "touchinput.h"
#include "ethdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , timeLabel(nullptr)
    , cpuLabel(nullptr)
    , memoryLabel(nullptr)
    , diskLabel(nullptr)
    , wifiButton(nullptr)
    , ethButton(nullptr)
    , wifiDialog(nullptr)
    , ethDialog(nullptr)
    , timeTimer(nullptr)
    , sysInfoTimer(nullptr)
{
    setWindowTitle("HelperBoard A133");
    
    resize(800, 480);
    setMaximumSize(800, 480);
    showFullScreen();

    QString logPath = QCoreApplication::applicationDirPath() + "/A133_BASE.log";
    logFile.setFileName(logPath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "\n========== Application Started at " 
            << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
            << " ==========\n";
        logFile.close();
    }

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 20, 10, 10);

    timeLabel = new QLabel(this);
    timeLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = timeLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    timeLabel->setFont(titleFont);
    mainLayout->addWidget(timeLabel);

    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    buttonLayout1->setSpacing(10);

    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    buttonLayout2->setSpacing(10);

    wifiButton = new QPushButton("[WiFi]", this);
    wifiButton->setMinimumSize(150, 60);
    wifiButton->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(wifiButton, &QPushButton::clicked, this, &MainWindow::openWifiSettings);
    buttonLayout1->addWidget(wifiButton);

    ethButton = new QPushButton("[ETH]", this);
    ethButton->setMinimumSize(150, 60);
    ethButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(ethButton, &QPushButton::clicked, this, &MainWindow::openEthSettings);
    buttonLayout1->addWidget(ethButton);

    QPushButton *terminalBtn = new QPushButton("[Terminal]", this);
    terminalBtn->setMinimumSize(150, 60);
    terminalBtn->setStyleSheet("QPushButton { background-color: #00BCD4; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(terminalBtn, &QPushButton::clicked, [this]() {
        writeLog("Opening Terminal");
        TerminalDialog dialog(this);
        dialog.exec();
        writeLog("Closed Terminal");
    });
    buttonLayout1->addWidget(terminalBtn);

    QPushButton *rebootBtn = new QPushButton("[Reboot]", this);
    rebootBtn->setMinimumSize(150, 60);
    rebootBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(rebootBtn, &QPushButton::clicked, this, &MainWindow::onReboot);
    buttonLayout1->addWidget(rebootBtn);

    QPushButton *clearLogBtn = new QPushButton("[Clear Log]", this);
    clearLogBtn->setMinimumSize(150, 60);
    clearLogBtn->setStyleSheet("QPushButton { background-color: #F44336; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLog);
    buttonLayout2->addWidget(clearLogBtn);

    QPushButton *cleanDiskBtn = new QPushButton("[Clean Disk]", this);
    cleanDiskBtn->setMinimumSize(150, 60);
    cleanDiskBtn->setStyleSheet("QPushButton { background-color: #795548; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(cleanDiskBtn, &QPushButton::clicked, this, &MainWindow::cleanDiskSpace);
    buttonLayout2->addWidget(cleanDiskBtn);

    mainLayout->addLayout(buttonLayout1);
    mainLayout->addLayout(buttonLayout2);
    mainLayout->addStretch();

    QHBoxLayout *sysInfoLayout = new QHBoxLayout();
    sysInfoLayout->setSpacing(15);
    sysInfoLayout->setContentsMargins(10, 5, 10, 5);

    cpuLabel = new QLabel("CPU: --%", this);
    QFont sysFont = cpuLabel->font();
    sysFont.setPointSize(11);
    cpuLabel->setFont(sysFont);
    cpuLabel->setStyleSheet("color: #1E88E5; font-weight: bold;");
    sysInfoLayout->addWidget(cpuLabel);

    memoryLabel = new QLabel("MEM: --%", this);
    memoryLabel->setFont(sysFont);
    memoryLabel->setStyleSheet("color: #43A047; font-weight: bold;");
    sysInfoLayout->addWidget(memoryLabel);

    diskLabel = new QLabel("DISK: --%", this);
    diskLabel->setFont(sysFont);
    diskLabel->setStyleSheet("color: #FB8C00; font-weight: bold;");
    sysInfoLayout->addWidget(diskLabel);

    mainLayout->addLayout(sysInfoLayout);

    setStyleSheet("QMainWindow { background-color: #ffffff; }");

    timeTimer = new QTimer(this);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start(1000);
    updateTime();

    sysInfoTimer = new QTimer(this);
    connect(sysInfoTimer, &QTimer::timeout, this, &MainWindow::updateSystemInfo);
    sysInfoTimer->start(2000);
    updateSystemInfo();

    writeLog("Main window initialized");
    qDebug() << "HelperBoard A133 started";
}

MainWindow::~MainWindow()
{
    writeLog("Application closed");
    
    if (timeTimer) {
        timeTimer->stop();
        delete timeTimer;
    }
    if (sysInfoTimer) {
        sysInfoTimer->stop();
        delete sysInfoTimer;
    }
    if (wifiDialog) {
        delete wifiDialog;
    }
    if (ethDialog) {
        delete ethDialog;
    }
}

void MainWindow::writeLog(const QString &message)
{
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "] " 
            << message << "\n";
        logFile.close();
    }
}

double MainWindow::getCPUUsage()
{
    QFile statFile("/proc/stat");
    if (!statFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream in(&statFile);
    QString line = in.readLine();
    statFile.close();

    if (!line.startsWith("cpu ")) {
        return -1;
    }

    QStringList parts = line.split(QRegExp("\\s+"));
    if (parts.size() < 5) {
        return -1;
    }

    bool ok;
    qulonglong user = parts[1].toULongLong(&ok);
    if (!ok) return -1;
    qulonglong nice = parts[2].toULongLong(&ok);
    if (!ok) return -1;
    qulonglong system = parts[3].toULongLong(&ok);
    if (!ok) return -1;
    qulonglong idle = parts[4].toULongLong(&ok);
    if (!ok) return -1;

    static qulonglong prevTotal = 0, prevIdle = 0;
    qulonglong total = user + nice + system + idle;
    qulonglong totalDiff = total - prevTotal;
    qulonglong idleDiff = idle - prevIdle;

    prevTotal = total;
    prevIdle = idle;

    if (totalDiff == 0) {
        return 0;
    }

    return ((totalDiff - idleDiff) * 100.0) / totalDiff;
}

double MainWindow::getMemoryUsage()
{
    QFile memFile("/proc/meminfo");
    if (!memFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream in(&memFile);
    QString line;
    qulonglong memTotal = 0, memAvailable = 0;

    while (!(line = in.readLine()).isNull()) {
        if (line.startsWith("MemTotal:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() >= 2) {
                bool ok;
                memTotal = parts[1].toULongLong(&ok);
            }
        } else if (line.startsWith("MemAvailable:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() >= 2) {
                bool ok;
                memAvailable = parts[1].toULongLong(&ok);
            }
        }
    }
    memFile.close();

    if (memTotal == 0) {
        return -1;
    }

    return ((memTotal - memAvailable) * 100.0) / memTotal;
}

double MainWindow::getDiskUsage()
{
    QProcess process;
    process.start("df", QStringList() << "-h" << "/");
    process.waitForFinished(1000);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    
    if (lines.size() >= 2) {
        QString dataLine = lines[1];
        QStringList parts = dataLine.split(QRegExp("\\s+"));
        if (parts.size() >= 5) {
            QString usage = parts[4];
            usage.remove("%");
            bool ok;
            double value = usage.toDouble(&ok);
            if (ok) {
                return value;
            }
        }
    }

    return -1;
}

void MainWindow::updateSystemInfo()
{
    double cpu = getCPUUsage();
    double mem = getMemoryUsage();
    double disk = getDiskUsage();

    if (cpu >= 0) {
        cpuLabel->setText(QString("CPU: %1%").arg(cpu, 0, 'f', 1));
    } else {
        cpuLabel->setText("CPU: --%");
    }

    if (mem >= 0) {
        memoryLabel->setText(QString("MEM: %1%").arg(mem, 0, 'f', 1));
    } else {
        memoryLabel->setText("MEM: --%");
    }

    if (disk >= 0) {
        diskLabel->setText(QString("DISK: %1%").arg(disk, 0, 'f', 1));
    } else {
        diskLabel->setText("DISK: --%");
    }
}

void MainWindow::updateTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QStringList weekDays;
    weekDays << "周日" << "周一" << "周二" << "周三" << "周四" << "周五" << "周六";
    int dayOfWeek = currentTime.date().dayOfWeek();
    QString weekDayText = weekDays[dayOfWeek - 1];
    QString timeText = currentTime.toString("yyyy-MM-dd ") + weekDayText + currentTime.toString(" hh:mm:ss");
    timeLabel->setText(timeText);
}

void MainWindow::openWifiSettings()
{
    writeLog("Opening WiFi settings");
    qDebug() << "Opening WiFi settings";

    if (!wifiDialog) {
        wifiDialog = new WifiDialog(this);
    }

    wifiDialog->show();
    wifiDialog->raise();
    wifiDialog->activateWindow();
}

void MainWindow::openEthSettings()
{
    writeLog("Opening ETH settings");
    qDebug() << "Opening ETH settings";

    if (!ethDialog) {
        ethDialog = new EthDialog(this);
    }

    ethDialog->show();
    ethDialog->raise();
    ethDialog->activateWindow();
}

void MainWindow::onReboot()
{
    writeLog("Reboot requested");
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Reboot");
    msgBox.setText("Are you sure you want to reboot the system?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        writeLog("Rebooting system...");
        qDebug() << "Rebooting system...";
        QMessageBox::information(this, "Reboot", "System will reboot now!");
        system("reboot");
    } else {
        writeLog("Reboot cancelled");
    }
}

void MainWindow::clearLog()
{
    QString logPath = QCoreApplication::applicationDirPath() + "/A133_BASE.log";
    QFile logFile(logPath);
    
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "========== Log Cleared at " 
            << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
            << " ==========\n";
        logFile.close();
    }
    
    writeLog("Log cleared by user");
    QMessageBox::information(this, "Clear Log", "Log file has been cleared!");
}

void MainWindow::cleanDiskSpace()
{
    writeLog("Disk cleanup requested");
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Disk Cleanup");
    msgBox.setText("This will clean up disk space by removing temporary files, cache, and old logs.\n\nAre you sure?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() != QMessageBox::Yes) {
        writeLog("Disk cleanup cancelled");
        return;
    }

    writeLog("Starting disk cleanup...");
    
    QStringList commands;
    commands << "rm -rf /tmp/*"
             << "rm -rf /var/tmp/*"
             << "rm -rf /var/log/*.old"
             << "rm -rf /var/log/*.log.*"
             << "find /var/log -type f -name '*.log' -mtime +7 -delete"
             << "apt-get clean"
             << "journalctl --vacuum-time=7d";

    QString result;
    foreach (const QString &cmd, commands) {
        QProcess process;
        process.start("bash", QStringList() << "-c" << cmd);
        process.waitForFinished(5000);
        
        QString output = process.readAllStandardOutput();
        QString error = process.readAllStandardError();
        
        if (!output.isEmpty()) {
            result += cmd + ":\n" + output + "\n";
        }
        if (!error.isEmpty()) {
            result += cmd + " (error):\n" + error + "\n";
        }
        
        writeLog("Executed: " + cmd);
    }

    writeLog("Disk cleanup completed");
    
    updateSystemInfo();
    
    QMessageBox::information(this, "Disk Cleanup", "Disk cleanup completed!\n\nDisk usage has been updated.");
}
