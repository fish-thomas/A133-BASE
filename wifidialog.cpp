#include "wifidialog.h"
#include "touchinput.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QGroupBox>
#include <QRegularExpression>
#include <QSettings>
#include <QSizePolicy>

WifiDialog::WifiDialog(QWidget *parent)
    : QDialog(parent)
    , scanProcess(nullptr)
    , connectProcess(nullptr)
    , disconnectProcess(nullptr)
    , currentSSID("")
    , currentIP("")
    , connected(false)
{
    setWindowTitle("WiFi Settings");
    resize(760, 440);
    setMaximumSize(760, 440);
    setStyleSheet("QDialog { background-color: #f5f5f5; }");

    setupUI();
    loadWifiSettings();

    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &WifiDialog::updateConnectionStatus);
    statusTimer->start(2000);

    updateConnectionStatus();

    QTimer::singleShot(500, this, &WifiDialog::scanWifi);
}

WifiDialog::~WifiDialog()
{
    if (statusTimer) {
        statusTimer->stop();
        delete statusTimer;
    }
    if (scanProcess) {
        scanProcess->kill();
        scanProcess->deleteLater();
    }
    if (connectProcess) {
        connectProcess->kill();
        connectProcess->deleteLater();
    }
    if (disconnectProcess) {
        disconnectProcess->kill();
        disconnectProcess->deleteLater();
    }
}

QString WifiDialog::getCurrentIP() const
{
    return currentIP;
}

QString WifiDialog::getCurrentSSID() const
{
    return currentSSID;
}

bool WifiDialog::isConnected() const
{
    return connected;
}

void WifiDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(6);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(4);

    connectionInfoLabel = new QLabel("Checking...", this);
    connectionInfoLabel->setAlignment(Qt::AlignCenter);
    connectionInfoLabel->setStyleSheet("padding: 6px; background-color: #e3f2fd; border-radius: 4px; font-size: 20px; font-weight: bold;");
    connectionInfoLabel->setMinimumHeight(40);
    leftLayout->addWidget(connectionInfoLabel);

    scanProgressBar = new QProgressBar(this);
    scanProgressBar->setVisible(false);
    scanProgressBar->setRange(0, 0);
    leftLayout->addWidget(scanProgressBar);

    wifiListWidget = new QListWidget(this);
    wifiListWidget->setStyleSheet("QListWidget { border: 1px solid #ccc; border-radius: 4px; padding: 2px; font-size: 14px; } QListWidget::item { padding: 6px; }");
    wifiListWidget->setMinimumHeight(120);
    connect(wifiListWidget, &QListWidget::itemClicked, this, &WifiDialog::onWifiItemClicked);
    leftLayout->addWidget(wifiListWidget);

    QGroupBox *connectGroup = new QGroupBox("Connect", this);
    connectGroup->setStyleSheet("QGroupBox { font-size: 13px; font-weight: bold; }");
    QVBoxLayout *connectLayout = new QVBoxLayout();
    connectLayout->setSpacing(4);
    connectLayout->setContentsMargins(6, 6, 6, 6);

    QHBoxLayout *ssidLayout = new QHBoxLayout();
    ssidEdit = new TouchInput(QString(), this);
    ssidEdit->setPlaceholderText("Enter SSID");
    ssidEdit->setStyleSheet("font-size: 14px; padding: 6px;");
    QLabel *ssidLabel = new QLabel("SSID:", this);
    ssidLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    ssidLayout->addWidget(ssidLabel);
    ssidLayout->addWidget(ssidEdit);
    connectLayout->addLayout(ssidLayout);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordEdit = new TouchInput(QString(), this);
    passwordEdit->setPlaceholderText("Enter password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet("font-size: 14px; padding: 6px;");
    QLabel *passwordLabel = new QLabel("Password:", this);
    passwordLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordEdit);

    showPasswordCheckbox = new QCheckBox("Show", this);
    showPasswordCheckbox->setStyleSheet("font-size: 12px;");
    connect(showPasswordCheckbox, &QCheckBox::toggled, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });
    passwordLayout->addWidget(showPasswordCheckbox);
    connectLayout->addLayout(passwordLayout);

    connectGroup->setLayout(connectLayout);
    leftLayout->addWidget(connectGroup);

    statusLabel = new QLabel("Ready", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("padding: 2px; color: #666; font-size: 20px;");
    statusLabel->setMinimumHeight(30);
    leftLayout->addWidget(statusLabel);

    leftLayout->addStretch();

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(4);

    scanButton = new QPushButton("Scan", this);
    scanButton->setMinimumSize(100, 50);
    scanButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    scanButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(scanButton, &QPushButton::clicked, this, &WifiDialog::scanWifi);
    rightLayout->addWidget(scanButton, 1);

    refreshButton = new QPushButton("Refresh", this);
    refreshButton->setMinimumSize(100, 50);
    refreshButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    refreshButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(refreshButton, &QPushButton::clicked, this, &WifiDialog::refreshWifiList);
    rightLayout->addWidget(refreshButton, 1);

    connectButton = new QPushButton("Connect", this);
    connectButton->setMinimumSize(100, 50);
    connectButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    connectButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(connectButton, &QPushButton::clicked, this, &WifiDialog::connectWifi);
    rightLayout->addWidget(connectButton, 1);

    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setMinimumSize(100, 50);
    disconnectButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    disconnectButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(disconnectButton, &QPushButton::clicked, this, &WifiDialog::disconnectWifi);
    rightLayout->addWidget(disconnectButton, 1);

    QPushButton *closeButton = new QPushButton("Close", this);
    closeButton->setMinimumSize(100, 50);
    closeButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    rightLayout->addWidget(closeButton, 1);

    contentLayout->addLayout(leftLayout, 4);
    contentLayout->addLayout(rightLayout, 1);

    mainLayout->addLayout(contentLayout);
}

void WifiDialog::saveWifiSettings(const QString &ssid, const QString &password)
{
    QSettings settings("helperboard", "rgb_display");
    settings.setValue("wifi/ssid", ssid);
    if (!password.isEmpty()) {
        settings.setValue("wifi/password", password);
    }
}

void WifiDialog::loadWifiSettings()
{
    QSettings settings("helperboard", "rgb_display");
    QString savedSSID = settings.value("wifi/ssid", "").toString();
    QString savedPassword = settings.value("wifi/password", "").toString();

    if (!savedSSID.isEmpty()) {
        ssidEdit->setText(savedSSID);
        passwordEdit->setText(savedPassword);
    }
}

bool WifiDialog::checkConnection()
{
    QString ip = getCurrentIPInternal();
    return !ip.isEmpty();
}

QString WifiDialog::getCurrentSSIDInternal()
{
    QProcess process;
    process.start("iwgetid", QStringList() << "-r");
    process.waitForFinished(2000);
    QString output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        return output;
    }

    process.start("iw", QStringList() << "dev" << "wlan0" << "link");
    process.waitForFinished(2000);
    output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("SSID:")) {
            QString ssid = line.split("SSID:").last().trimmed();
            if (!ssid.isEmpty()) {
                return ssid;
            }
        }
    }

    return "Connected";
}

QString WifiDialog::getCurrentIPInternal()
{
    QProcess process;
    process.start("ip", QStringList() << "addr" << "show" << "wlan0");
    process.waitForFinished(2000);
    QString output = process.readAllStandardOutput();

    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("inet ") && !line.contains("inet6")) {
            QStringList parts = line.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "inet") {
                    QString ipWithMask = parts[i + 1];
                    return ipWithMask.split('/').first();
                }
            }
        }
    }
    return "";
}

void WifiDialog::scanWifi()
{
    statusLabel->setText("Scanning...");
    scanProgressBar->setVisible(true);
    scanButton->setEnabled(false);
    refreshButton->setEnabled(false);

    if (scanProcess) {
        scanProcess->kill();
        scanProcess->deleteLater();
    }

    scanProcess = new QProcess(this);
    connect(scanProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WifiDialog::onScanFinished);

    QStringList args;
    args << "-t" << "-f" << "SSID,SIGNAL" << "device" << "wifi" << "list" << "--rescan" << "yes";
    scanProcess->start("nmcli", args);

    QTimer::singleShot(8000, [this]() {
        if (scanProcess && scanProcess->state() == QProcess::Running) {
            scanProcess->kill();
            scanProgressBar->setVisible(false);
            scanButton->setEnabled(true);
            refreshButton->setEnabled(true);
        }
    });
}

void WifiDialog::showMockNetworks()
{
    wifiListWidget->clear();

    QStringList mockNetworks = {"HomeWiFi", "OfficeNet", "GuestNetwork", "FreeWiFi", "MyWiFi_2.4G", "MyWiFi_5G"};
    QList<int> mockSignals = {-35, -45, -55, -65, -70, -75};

    for (int i = 0; i < mockNetworks.size(); ++i) {
        QString icon = getSignalStrengthIcon(mockSignals[i]);
        QString itemText = QString("%1 %2").arg(icon, mockNetworks[i]);
        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, mockNetworks[i]);
        wifiListWidget->addItem(item);
    }
}

void WifiDialog::onScanFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString output = scanProcess->readAllStandardOutput();

    if (exitStatus != QProcess::NormalExit || exitCode != 0 || output.isEmpty()) {
        if (scanProcess) {
            scanProcess->deleteLater();
        }

        scanProcess = new QProcess(this);
        connect(scanProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this](int code, QProcess::ExitStatus status) {
                    scanProgressBar->setVisible(false);
                    scanButton->setEnabled(true);
                    refreshButton->setEnabled(true);

                    if (status == QProcess::NormalExit && code == 0) {
                        parseScanResults(scanProcess->readAllStandardOutput());
                    } else {
                        showMockNetworks();
                    }
                });

        scanProcess->start("iwlist", QStringList() << "wlan0" << "scan");
        return;
    }

    scanProgressBar->setVisible(false);
    scanButton->setEnabled(true);
    refreshButton->setEnabled(true);

    parseScanResults(output);
    if (wifiListWidget->count() == 0) {
        showMockNetworks();
    }
}

void WifiDialog::parseScanResults(const QString &output)
{
    wifiListWidget->clear();

    QStringList lines = output.split('\n');
    bool foundNetworks = false;

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;

        if (trimmed.startsWith('"') && trimmed.endsWith('"')) {
            trimmed = trimmed.mid(1, trimmed.length() - 2);
        }

        int colonPos = trimmed.lastIndexOf(':');
        if (colonPos > 0 && colonPos < trimmed.length() - 1) {
            QString ssid = trimmed.left(colonPos);
            QString signalStr = trimmed.mid(colonPos + 1);

            bool ok;
            int signalPercent = signalStr.toInt(&ok);
            if (ok && signalPercent >= 0 && signalPercent <= 100) {
                QString icon = getSignalStrengthIcon(-100 + signalPercent);
                QString itemText = QString("%1 %2 (%3%)").arg(icon, ssid, QString::number(signalPercent));
                QListWidgetItem *item = new QListWidgetItem(itemText);
                item->setData(Qt::UserRole, ssid);
                wifiListWidget->addItem(item);
                foundNetworks = true;
            }
        }
    }

    if (foundNetworks) return;

    QString currentSSID;
    QString currentSignal;
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        if (trimmed.startsWith("ESSID:")) {
            currentSSID = trimmed.mid(7).remove('"');
        } else if (trimmed.startsWith("Signal level=")) {
            currentSignal = trimmed.mid(13);
            int signalEnd = currentSignal.indexOf(' ');
            if (signalEnd > 0) {
                currentSignal = currentSignal.left(signalEnd);
            }

            if (!currentSSID.isEmpty()) {
                QString icon = getSignalStrengthIcon(currentSignal.toInt());
                QString itemText = QString("%1 %2").arg(icon, currentSSID);
                QListWidgetItem *item = new QListWidgetItem(itemText);
                item->setData(Qt::UserRole, currentSSID);
                wifiListWidget->addItem(item);
            }
            currentSSID.clear();
        }
    }
}

QString WifiDialog::getSignalStrengthIcon(int signalDb)
{
    if (signalDb >= -50) return "📶📶📶📶";
    if (signalDb >= -60) return "📶📶📶";
    if (signalDb >= -70) return "📶📶";
    if (signalDb >= -80) return "📶";
    return "📶";
}

void WifiDialog::onWifiItemClicked(QListWidgetItem *item)
{
    QString ssid = item->data(Qt::UserRole).toString();
    if (!ssid.isEmpty()) {
        ssidEdit->setText(ssid);
    }
}

void WifiDialog::connectWifi()
{
    QString ssid = ssidEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (ssid.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter SSID!");
        return;
    }

    statusLabel->setText("Connecting...");
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(false);

    if (connectProcess) {
        connectProcess->kill();
        connectProcess->deleteLater();
    }

    connectProcess = new QProcess(this);
    connect(connectProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WifiDialog::onConnectFinished);

    QStringList args;
    if (password.isEmpty()) {
        args << "device" << "wifi" << "connect" << ssid;
    } else {
        args << "device" << "wifi" << "connect" << ssid << "password" << password;
    }
    connectProcess->start("nmcli", args);

    QTimer::singleShot(15000, [this]() {
        if (connectProcess && connectProcess->state() == QProcess::Running) {
            connectProcess->kill();
            statusLabel->setText("Timeout");
            connectButton->setEnabled(true);
            disconnectButton->setEnabled(true);
        }
    });
}

void WifiDialog::onConnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(true);

    if (exitStatus == QProcess::NormalExit) {
        saveWifiSettings(ssidEdit->text(), passwordEdit->text());

        QTimer::singleShot(3000, [this]() {
            QString ip = getCurrentIPInternal();
            QString ssid = getCurrentSSIDInternal();

            if (!ip.isEmpty()) {
                currentSSID = ssid;
                currentIP = ip;
                connected = true;
                statusLabel->setText(QString("IP: %1").arg(ip));
                QMessageBox::information(this, "Connected", QString("SSID: %1\nIP: %2").arg(ssid, ip));
                emit wifiStatusChanged(true, ssid, ip);
            } else {
                statusLabel->setText("Connected");
                currentSSID = ssid;
                emit wifiStatusChanged(true, ssid, "");
            }
            updateConnectionStatus();
        });
    } else {
        statusLabel->setText("Failed");
        QMessageBox::warning(this, "Failed", "Connection failed");
    }
}

void WifiDialog::disconnectWifi()
{
    statusLabel->setText("Disconnecting...");
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(false);

    if (disconnectProcess) {
        disconnectProcess->kill();
        disconnectProcess->deleteLater();
    }

    disconnectProcess = new QProcess(this);
    connect(disconnectProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WifiDialog::onDisconnectFinished);

    disconnectProcess->start("nmcli", QStringList() << "device" << "disconnect" << "wlan0");
}

void WifiDialog::onDisconnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(true);

    if (exitStatus == QProcess::NormalExit) {
        currentSSID = "";
        currentIP = "";
        connected = false;
        statusLabel->setText("Disconnected");
        QMessageBox::information(this, "Disconnected", "WiFi disconnected");
        emit wifiStatusChanged(false, "", "");
        updateConnectionStatus();
    } else {
        statusLabel->setText("Disconnect failed");
    }
}

void WifiDialog::updateConnectionStatus()
{
    bool isConnected = checkConnection();
    QString ssid = getCurrentSSIDInternal();
    QString ip = getCurrentIPInternal();

    if (isConnected && !ssid.isEmpty()) {
        currentSSID = ssid;
        currentIP = ip;
        connected = true;

        if (!ip.isEmpty()) {
            connectionInfoLabel->setText(QString("✅ %1 | IP: %2").arg(ssid, ip));
        } else {
            connectionInfoLabel->setText(QString("✅ %1").arg(ssid));
        }
        connectionInfoLabel->setStyleSheet("padding: 6px; background-color: #e8f5e9; color: #2e7d32; border-radius: 4px; font-size: 20px; font-weight: bold;");
        emit wifiStatusChanged(true, ssid, ip);
    } else {
        currentSSID = "";
        currentIP = "";
        connected = false;
        connectionInfoLabel->setText("❌ Not connected");
        connectionInfoLabel->setStyleSheet("padding: 6px; background-color: #ffebee; color: #c62828; border-radius: 4px; font-size: 20px; font-weight: bold;");
        emit wifiStatusChanged(false, "", "");
    }
}

void WifiDialog::refreshWifiList()
{
    scanWifi();
}
