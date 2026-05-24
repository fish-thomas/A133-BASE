#include "ethdialog.h"
#include <QDebug>
#include <QLabel>
#include <QSizePolicy>

EthDialog::EthDialog(QWidget *parent)
    : QDialog(parent)
    , statusTimer(nullptr)
{
    setWindowTitle("Ethernet Settings");
    resize(760, 440);
    setMaximumSize(760, 440);
    setStyleSheet("QDialog { background-color: #f5f5f5; }");

    setupUI();

    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &EthDialog::updateEthStatus);
    statusTimer->start(3000);

    updateEthStatus();
}

EthDialog::~EthDialog()
{
    if (statusTimer) {
        statusTimer->stop();
        delete statusTimer;
    }
}

void EthDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(6);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(4);

    connectionStatusLabel = new QLabel("Checking...", this);
    connectionStatusLabel->setAlignment(Qt::AlignCenter);
    connectionStatusLabel->setStyleSheet("padding: 6px; background-color: #e3f2fd; border-radius: 4px; font-size: 40px; font-weight: bold;");
    connectionStatusLabel->setMinimumHeight(60);
    leftLayout->addWidget(connectionStatusLabel);

    QGroupBox *infoGroup = new QGroupBox("Ethernet Information", this);
    infoGroup->setStyleSheet("QGroupBox { font-size: 26px; font-weight: bold; }");
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(6);
    infoLayout->setContentsMargins(6, 6, 6, 6);

    ipAddressLabel = new QLabel("IP Address: --", this);
    ipAddressLabel->setStyleSheet("padding: 6px; font-size: 28px; background-color: white; border: 1px solid #ddd; border-radius: 3px;");
    infoLayout->addWidget(ipAddressLabel);

    macAddressLabel = new QLabel("MAC Address: --", this);
    macAddressLabel->setStyleSheet("padding: 6px; font-size: 28px; background-color: white; border: 1px solid #ddd; border-radius: 3px;");
    infoLayout->addWidget(macAddressLabel);

    netmaskLabel = new QLabel("Netmask: --", this);
    netmaskLabel->setStyleSheet("padding: 6px; font-size: 28px; background-color: white; border: 1px solid #ddd; border-radius: 3px;");
    infoLayout->addWidget(netmaskLabel);

    gatewayLabel = new QLabel("Gateway: --", this);
    gatewayLabel->setStyleSheet("padding: 6px; font-size: 28px; background-color: white; border: 1px solid #ddd; border-radius: 3px;");
    infoLayout->addWidget(gatewayLabel);

    dnsLabel = new QLabel("DNS: --", this);
    dnsLabel->setStyleSheet("padding: 6px; font-size: 28px; background-color: white; border: 1px solid #ddd; border-radius: 3px;");
    infoLayout->addWidget(dnsLabel);

    infoGroup->setLayout(infoLayout);
    leftLayout->addWidget(infoGroup);

    ethInfoLabel = new QLabel("", this);
    ethInfoLabel->setStyleSheet("padding: 6px; font-size: 24px; font-family: monospace; background-color: #f0f0f0; border-radius: 3px;");
    ethInfoLabel->setWordWrap(true);
    leftLayout->addWidget(ethInfoLabel);

    leftLayout->addStretch();

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(4);

    QPushButton *refreshButton = new QPushButton("Refresh", this);
    refreshButton->setMinimumSize(100, 50);
    refreshButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 6px; border: none; border-radius: 6px; font-size: 16px; font-weight: bold; }");
    refreshButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(refreshButton, &QPushButton::clicked, this, &EthDialog::updateEthStatus);
    rightLayout->addWidget(refreshButton, 1);

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

void EthDialog::updateEthStatus()
{
    QStringList ethInterfaces = {"eth0", "eth1", "end0"};
    QString foundInterface;
    QString ethInfo;

    for (const QString &iface : ethInterfaces) {
        ethInfo = getEthInfo(iface);
        if (!ethInfo.isEmpty()) {
            foundInterface = iface;
            break;
        }
    }

    if (foundInterface.isEmpty()) {
        ethInfoLabel->setText("No Ethernet interface found");
        connectionStatusLabel->setText("❌ Ethernet Disconnected");
        connectionStatusLabel->setStyleSheet("padding: 6px; font-size: 40px; font-weight: bold; background-color: #ffebee; color: #c62828; border-radius: 4px;");

        ipAddressLabel->setText("IP Address: N/A");
        macAddressLabel->setText("MAC Address: N/A");
        netmaskLabel->setText("Netmask: N/A");
        gatewayLabel->setText("Gateway: N/A");
        dnsLabel->setText("DNS: N/A");
        return;
    }

    ethInfoLabel->setText(ethInfo);

    QProcess process;
    process.start("ip", QStringList() << "addr" << "show" << foundInterface);
    process.waitForFinished(2000);
    QString output = process.readAllStandardOutput();

    QString currentIP, currentMAC, currentNetmask;

    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("inet ")) {
            QStringList parts = line.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "inet") {
                    QString ipWithMask = parts[i + 1];
                    currentIP = ipWithMask.split('/').first();
                    currentNetmask = ipWithMask.mid(ipWithMask.indexOf('/') + 1);
                    break;
                }
            }
        } else if (line.contains("ether ")) {
            QStringList parts = line.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "ether") {
                    currentMAC = parts[i + 1];
                    break;
                }
            }
        }
    }

    QProcess routeProcess;
    routeProcess.start("ip", QStringList() << "route" << "show");
    routeProcess.waitForFinished(2000);
    QString routeOutput = routeProcess.readAllStandardOutput();
    QString gateway;
    QStringList routeLines = routeOutput.split('\n');
    for (const QString &routeLine : routeLines) {
        if (routeLine.startsWith("default")) {
            QStringList parts = routeLine.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "via") {
                    gateway = parts[i + 1];
                    break;
                }
            }
            break;
        }
    }

    QProcess dnsProcess;
    dnsProcess.start("cat", QStringList() << "/etc/resolv.conf");
    dnsProcess.waitForFinished(2000);
    QString dnsOutput = dnsProcess.readAllStandardOutput();
    QString dns;
    QStringList dnsLines = dnsOutput.split('\n');
    for (const QString &dnsLine : dnsLines) {
        if (dnsLine.startsWith("nameserver")) {
            QStringList parts = dnsLine.trimmed().split(' ');
            if (parts.size() >= 2) {
                if (!dns.isEmpty()) dns += ", ";
                dns += parts[1];
            }
        }
    }

    if (!currentIP.isEmpty()) {
        connectionStatusLabel->setText(QString("✅ Ethernet Connected (%1)").arg(foundInterface));
        connectionStatusLabel->setStyleSheet("padding: 6px; font-size: 40px; font-weight: bold; background-color: #e8f5e9; color: #2e7d32; border-radius: 4px;");

        ipAddressLabel->setText(QString("IP Address: %1").arg(currentIP));
        macAddressLabel->setText(QString("MAC Address: %1").arg(currentMAC));
        netmaskLabel->setText(QString("Netmask: %1").arg(currentNetmask));
        gatewayLabel->setText(QString("Gateway: %1").arg(gateway.isEmpty() ? "N/A" : gateway));
        dnsLabel->setText(QString("DNS: %1").arg(dns.isEmpty() ? "N/A" : dns));
    } else {
        connectionStatusLabel->setText(QString("❌ Ethernet Disconnected (%1)").arg(foundInterface));
        connectionStatusLabel->setStyleSheet("padding: 6px; font-size: 40px; font-weight: bold; background-color: #ffebee; color: #c62828; border-radius: 4px;");

        ipAddressLabel->setText("IP Address: N/A");
        macAddressLabel->setText(QString("MAC Address: %1").arg(currentMAC.isEmpty() ? "N/A" : currentMAC));
        netmaskLabel->setText("Netmask: N/A");
        gatewayLabel->setText("Gateway: N/A");
        dnsLabel->setText("DNS: N/A");
    }
}

QString EthDialog::getEthInfo(const QString &interface)
{
    QProcess process;
    process.start("ip", QStringList() << "addr" << "show" << interface);
    process.waitForFinished(2000);
    QString output = process.readAllStandardOutput();

    if (output.isEmpty() || !output.contains("inet ")) {
        return QString();
    }

    QString result = QString("Interface: %1\n").arg(interface);

    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("inet ")) {
            QStringList parts = line.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "inet") {
                    result += QString("  IPv4: %1\n").arg(parts[i + 1]);
                    break;
                }
            }
        } else if (line.contains("ether ")) {
            QStringList parts = line.trimmed().split(' ');
            for (int i = 0; i < parts.size(); ++i) {
                if (parts[i] == "ether") {
                    result += QString("  MAC: %1\n").arg(parts[i + 1]);
                    break;
                }
            }
        }
    }

    return result;
}
