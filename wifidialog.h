#ifndef WIFIDIALOG_H
#define WIFIDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QProcess>
#include <QCheckBox>
#include <QLineEdit>

class TouchInput;

class WifiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WifiDialog(QWidget *parent = nullptr);
    ~WifiDialog();

    QString getCurrentIP() const;
    QString getCurrentSSID() const;
    bool isConnected() const;

signals:
    void wifiStatusChanged(bool connected, const QString &ssid, const QString &ip);

private slots:
    void scanWifi();
    void connectWifi();
    void disconnectWifi();
    void onScanFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConnectFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDisconnectFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onWifiItemClicked(QListWidgetItem *item);
    void updateConnectionStatus();
    void refreshWifiList();

private:
    void setupUI();
    void parseScanResults(const QString &output);
    void showMockNetworks();
    QString getSignalStrengthIcon(int signalDb);
    void saveWifiSettings(const QString &ssid, const QString &password);
    void loadWifiSettings();
    bool checkConnection();
    QString getCurrentIPInternal();
    QString getCurrentSSIDInternal();

    QListWidget *wifiListWidget;
    QLineEdit *ssidEdit;
    QLineEdit *passwordEdit;
    QPushButton *scanButton;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *refreshButton;
    QLabel *statusLabel;
    QLabel *connectionInfoLabel;
    QProgressBar *scanProgressBar;
    QCheckBox *showPasswordCheckbox;

    QProcess *scanProcess;
    QProcess *connectProcess;
    QProcess *disconnectProcess;
    QTimer *statusTimer;

    QString selectedSSID;
    QStringList knownNetworks;
    QString currentSSID;
    QString currentIP;
    bool connected;
};

#endif // WIFIDIALOG_H
