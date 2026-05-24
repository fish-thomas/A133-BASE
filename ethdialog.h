#ifndef ETHDIALOG_H
#define ETHDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QProcess>
#include <QGroupBox>

class EthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EthDialog(QWidget *parent = nullptr);
    ~EthDialog();

private slots:
    void updateEthStatus();

private:
    void setupUI();
    QString getEthInfo(const QString &interface);

    QLabel *ethInfoLabel;
    QLabel *connectionStatusLabel;
    QLabel *ipAddressLabel;
    QLabel *macAddressLabel;
    QLabel *netmaskLabel;
    QLabel *gatewayLabel;
    QLabel *dnsLabel;
    QTimer *statusTimer;
};

#endif // ETHDIALOG_H
