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
#include <QProcessEnvironment>
#include <QFile>
#include <QCoreApplication>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>

// 版本信息
const QString APP_VERSION = "1.2.0";

// 更新日志
const QString UPDATE_LOG = 
    "v1.2.0 (2026-05-24):\n"
    "  - 添加 Update 功能，支持从 GitHub 下载代码\n"
    "  - 添加 Build & Update 功能，自动编译更新并重启\n"
    "  - 添加 View Logs 按钮，方便查看日志\n"
    "  - 添加 Clean Disk 功能，清理磁盘空间\n"
    "  - 日志文件路径改为程序目录，文件名改为 A133_BASE.log\n"
    "  - 主界面按钮字体放大一倍\n"
    "  - 项目名称从 rgb_display_demo 改为 A133_BASE\n"
    "\n"
    "v1.1.0:\n"
    "  - 添加 Terminal 终端功能\n"
    "  - 添加 Reboot 重启功能\n"
    "  - 添加系统信息显示（CPU/内存/磁盘）\n"
    "  - 添加日志保存和 Clear Log 功能\n"
    "  - 界面适配 800x480 分辨率\n"
    "\n"
    "v1.0.0:\n"
    "  - 初始版本\n"
    "  - WiFi 设置功能\n"
    "  - ETH 以太网设置功能\n"
    "  - 触摸屏虚拟键盘\n";

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

    // 版本信息
    QLabel *versionLabel = new QLabel("A133_BASE v" + APP_VERSION, this);
    versionLabel->setAlignment(Qt::AlignCenter);
    QFont versionFont = versionLabel->font();
    versionFont.setPointSize(12);
    versionFont.setBold(true);
    versionLabel->setFont(versionFont);
    versionLabel->setStyleSheet("color: #2196F3;");
    mainLayout->addWidget(versionLabel);

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

    QHBoxLayout *buttonLayout3 = new QHBoxLayout();
    buttonLayout3->setSpacing(10);

    QPushButton *updateBtn = new QPushButton("[Update]", this);
    updateBtn->setMinimumSize(150, 60);
    updateBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(updateBtn, &QPushButton::clicked, this, &MainWindow::updateCode);
    buttonLayout3->addWidget(updateBtn);

    QPushButton *viewLogsBtn = new QPushButton("[View Logs]", this);
    viewLogsBtn->setMinimumSize(150, 60);
    viewLogsBtn->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 28px; font-weight: bold; }");
    connect(viewLogsBtn, &QPushButton::clicked, this, &MainWindow::viewLogs);
    buttonLayout3->addWidget(viewLogsBtn);

    mainLayout->addLayout(buttonLayout1);
    mainLayout->addLayout(buttonLayout2);
    mainLayout->addLayout(buttonLayout3);
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
    
    // Update Log 按钮
    QHBoxLayout *updateLogBtnLayout = new QHBoxLayout();
    updateLogBtnLayout->setContentsMargins(10, 0, 10, 0);
    QPushButton *updateLogBtn = new QPushButton("Update Log", this);
    updateLogBtn->setMinimumSize(150, 35);
    updateLogBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; }");
    connect(updateLogBtn, &QPushButton::clicked, this, &MainWindow::showUpdateLog);
    updateLogBtnLayout->addStretch();
    updateLogBtnLayout->addWidget(updateLogBtn);
    mainLayout->addLayout(updateLogBtnLayout);

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

void MainWindow::updateCode()
{
    writeLog("Code update requested");
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Update");
    msgBox.setText("Update code from GitHub?\nRepo: https://github.com/fish-thomas/A133-BASE.git");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    // 设置对话框宽度以适应屏幕
    msgBox.setMinimumWidth(700);
    msgBox.setMaximumSize(780, 400);

    if (msgBox.exec() != QMessageBox::Yes) {
        writeLog("Code update cancelled");
        return;
    }

    writeLog("Starting code update...");
    
    QString updateDir = QDir::homePath() + "/A133_BASE_update";
    QString repoUrl = "https://github.com/fish-thomas/A133-BASE.git";
    
    QProcess process;
    
    // 检查 git 是否可用
    process.start("git", QStringList() << "--version");
    if (!process.waitForFinished(5000)) {
        writeLog("Error: git not found or timeout");
        QMessageBox::warning(this, "Update Error", "git command not found!");
        return;
    }
    
    // 取消 git 代理设置
    writeLog("Clearing git proxy settings");
    process.start("git", QStringList() << "config" << "--global" << "http.proxy");
    process.waitForFinished(3000);
    process.start("git", QStringList() << "config" << "--global" << "https.proxy");
    process.waitForFinished(3000);
    process.start("git", QStringList() << "config" << "--global" << "--unset" << "http.proxy");
    process.waitForFinished(3000);
    process.start("git", QStringList() << "config" << "--global" << "--unset" << "https.proxy");
    process.waitForFinished(3000);
    process.start("git", QStringList() << "config" << "--unset" << "http.proxy");
    process.waitForFinished(3000);
    process.start("git", QStringList() << "config" << "--unset" << "https.proxy");
    process.waitForFinished(3000);
    
    // 清理旧的更新目录（如果存在）
    QDir dir(updateDir);
    if (dir.exists()) {
        writeLog("Removing old update directory");
        dir.removeRecursively();
    }
    
    // 克隆仓库（禁用 SSL 验证以避免证书问题
    writeLog("Cloning repository from " + repoUrl);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GIT_SSL_NO_VERIFY", "1");
    process.setProcessEnvironment(env);
    process.start("git", QStringList() << "clone" << repoUrl << updateDir);
    
    bool finished = process.waitForFinished(180000); // 增加到3分钟，网络慢的话需要更长时间
    
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (!output.isEmpty()) {
        writeLog("Clone output:\n" + output);
    }
    if (!error.isEmpty()) {
        writeLog("Clone error:\n" + error);
    }
    
    bool success = false;
    
    if (!finished) {
        writeLog("ERROR: Clone timed out after 3 minutes!");
        process.kill();
    } else if (process.exitCode() != 0) {
        writeLog("Repository clone failed with exit code: " + QString::number(process.exitCode()));
    } else {
        // 检查目录是否真的存在并包含文件
        QDir checkDir(updateDir);
        if (checkDir.exists() && checkDir.entryList(QDir::Files | QDir::NoDotAndDotDot).size() > 0) {
            success = true;
            writeLog("Repository cloned successfully!");
        } else {
            writeLog("ERROR: Clone reported success but directory is missing or empty!");
        }
    }
    
    if (success) {
        writeLog("Code update completed successfully");
        
        // 创建自定义成功对话框
        QDialog successDialog(this);
        successDialog.setWindowTitle("Update Complete");
        successDialog.resize(700, 250);
        successDialog.setMaximumSize(780, 300);
        
        QVBoxLayout *dialogLayout = new QVBoxLayout(&successDialog);
        dialogLayout->setContentsMargins(20, 20, 20, 20);
        dialogLayout->setSpacing(20);
        
        QLabel *infoLabel = new QLabel("Code downloaded successfully!\n\nLocation: " + updateDir + "\n\nClick 'Run' to build and update.", &successDialog);
        infoLabel->setWordWrap(true);
        infoLabel->setAlignment(Qt::AlignCenter);
        QFont labelFont = infoLabel->font();
        labelFont.setPointSize(12);
        infoLabel->setFont(labelFont);
        dialogLayout->addWidget(infoLabel);
        
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(20);
        
        QPushButton *okBtn = new QPushButton("OK", &successDialog);
        okBtn->setMinimumSize(120, 50);
        okBtn->setStyleSheet("QPushButton { background-color: #607D8B; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 18px; font-weight: bold; }");
        
        QPushButton *runBtn = new QPushButton("Run", &successDialog);
        runBtn->setMinimumSize(120, 50);
        runBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 18px; font-weight: bold; }");
        
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(runBtn);
        btnLayout->addStretch();
        dialogLayout->addLayout(btnLayout);
        
        // 连接按钮
        connect(okBtn, &QPushButton::clicked, &successDialog, &QDialog::accept);
        
        // Run 按钮点击后关闭对话框并执行更新
        connect(runBtn, &QPushButton::clicked, [this, &successDialog, updateDir]() {
            successDialog.accept();
            buildAndUpdateFromDir(updateDir);
        });
        
        successDialog.exec();
    } else {
        writeLog("Error: " + error);
        writeLog("Output: " + output);
        
        QMessageBox::warning(this, "Update Failed", 
            "Failed to update code!\n\nPlease check the log file for details.");
    }
}

void MainWindow::viewLogs()
{
    writeLog("Viewing logs");
    
    QString logPath = QCoreApplication::applicationDirPath() + "/A133_BASE.log";
    QFile logFile(logPath);
    
    QString logContent;
    
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logFile);
        logContent = in.readAll();
        logFile.close();
    } else {
        logContent = "Log file not found: " + logPath;
    }
    
    // 创建一个对话框来显示日志
    QDialog logDialog(this);
    logDialog.setWindowTitle("Log Viewer");
    logDialog.resize(760, 420);
    logDialog.setMaximumSize(780, 440);
    
    QVBoxLayout *layout = new QVBoxLayout(&logDialog);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    QTextEdit *textEdit = new QTextEdit(&logDialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(logContent);
    QFont font = textEdit->font();
    font.setFamily("Courier New");
    font.setPointSize(10);
    textEdit->setFont(font);
    layout->addWidget(textEdit);
    
    QPushButton *closeBtn = new QPushButton("Close", &logDialog);
    closeBtn->setMinimumSize(120, 40);
    closeBtn->setStyleSheet("QPushButton { background-color: #607D8B; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }");
    connect(closeBtn, &QPushButton::clicked, &logDialog, &QDialog::accept);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
    
    logDialog.exec();
}

void MainWindow::buildAndUpdateFromDir(const QString &updateDir)
{
    writeLog("Build & Update requested");
    
    QString currentDir = QCoreApplication::applicationDirPath();
    
    // 检查更新目录是否存在
    QDir dir(updateDir);
    if (!dir.exists()) {
        writeLog("ERROR: Update directory not found: " + updateDir);
        QMessageBox::warning(this, "Update Failed", 
            "Update directory not found!");
        return;
    }
    
    writeLog("Starting build process...");
    
    // 创建 build 目录
    QString buildDir = updateDir + "/build";
    QDir buildQDir(buildDir);
    if (buildQDir.exists()) {
        writeLog("Removing old build directory");
        buildQDir.removeRecursively();
    }
    buildQDir.mkpath(".");
    
    QProcess process;
    
    // 1. 找到 .pro 文件（可能是 A133_BASE.pro 或 rgb_display_demo.pro）
    QString proFile;
    QStringList proFiles = dir.entryList(QStringList() << "*.pro", QDir::Files);
    if (proFiles.isEmpty()) {
        writeLog("ERROR: No .pro file found in update directory!");
        QMessageBox::warning(this, "Build Failed", "No .pro file found!");
        return;
    }
    proFile = proFiles.first();
    writeLog("Found .pro file: " + proFile);
    
    // 2. 运行 qmake
    writeLog("Running qmake...");
    process.setWorkingDirectory(buildDir);
    process.start("qmake", QStringList() << "../" + proFile);
    if (!process.waitForFinished(30000)) {
        writeLog("ERROR: qmake timed out!");
        QMessageBox::warning(this, "Build Failed", "qmake timed out!");
        return;
    }
    if (process.exitCode() != 0) {
        writeLog("ERROR: qmake failed with exit code " + QString::number(process.exitCode()));
        writeLog("qmake error: " + process.readAllStandardError());
        QMessageBox::warning(this, "Build Failed", "qmake failed!\n\nCheck log for details.");
        return;
    }
    writeLog("qmake completed successfully");
    
    // 3. 运行 make
    writeLog("Running make...");
    process.start("make", QStringList() << "-j2");
    if (!process.waitForFinished(180000)) {
        writeLog("ERROR: make timed out!");
        QMessageBox::warning(this, "Build Failed", "make timed out!");
        return;
    }
    if (process.exitCode() != 0) {
        writeLog("ERROR: make failed with exit code " + QString::number(process.exitCode()));
        writeLog("make error: " + process.readAllStandardError());
        QMessageBox::warning(this, "Build Failed", "make failed!\n\nCheck log for details.");
        return;
    }
    writeLog("make completed successfully");
    
    // 4. 检查可执行文件（可能是 A133_BASE 或 rgb_display_demo）
    QString newBinary;
    QStringList binaryNames;
    binaryNames << "A133_BASE" << "rgb_display_demo";
    
    QDir buildDirCheck(buildDir);
    QStringList files = buildDirCheck.entryList(QDir::Files | QDir::Executable);
    
    bool found = false;
    for (const QString &binName : binaryNames) {
        if (files.contains(binName)) {
            newBinary = buildDir + "/" + binName;
            found = true;
            break;
        }
    }
    
    if (!found) {
        writeLog("ERROR: New binary not found in build directory!");
        QMessageBox::warning(this, "Build Failed", "New binary not found!");
        return;
    }
    writeLog("Found binary: " + newBinary);
    
    // 5. 创建更新脚本
    writeLog("Creating update script...");
    QString scriptPath = updateDir + "/update_script.sh";
    QFile scriptFile(scriptPath);
    if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&scriptFile);
        out << "#!/bin/bash\n";
        out << "sleep 2\n";
        out << "cd \"" << currentDir << "\"\n";
        out << "cp \"" << newBinary << "\" ./A133_BASE\n";
        out << "chmod +x ./A133_BASE\n";
        out << "exec ./A133_BASE\n";
        scriptFile.close();
        
        // 设置执行权限
        process.start("chmod", QStringList() << "+x" << scriptPath);
        process.waitForFinished(5000);
    }
    
    writeLog("Build completed successfully! Restarting...");
    
    QMessageBox::information(this, "Update Complete", 
        "Build completed! Program will restart now.");
    
    // 6. 执行更新脚本并退出
    process.startDetached(scriptPath);
    QCoreApplication::quit();
}

void MainWindow::showUpdateLog()
{
    writeLog("Showing update log");
    
    QDialog logDialog(this);
    logDialog.setWindowTitle("Update Log");
    logDialog.resize(750, 400);
    logDialog.setMaximumSize(780, 440);
    
    QVBoxLayout *layout = new QVBoxLayout(&logDialog);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);
    
    QTextEdit *textEdit = new QTextEdit(&logDialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(UPDATE_LOG);
    QFont font = textEdit->font();
    font.setFamily("Courier New");
    font.setPointSize(10);
    textEdit->setFont(font);
    layout->addWidget(textEdit);
    
    QPushButton *closeBtn = new QPushButton("Close", &logDialog);
    closeBtn->setMinimumSize(120, 40);
    closeBtn->setStyleSheet("QPushButton { background-color: #607D8B; color: white; padding: 8px; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }");
    connect(closeBtn, &QPushButton::clicked, &logDialog, &QDialog::accept);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
    
    logDialog.exec();
}
