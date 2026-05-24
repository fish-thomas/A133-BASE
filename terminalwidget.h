#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QKeyEvent>
#include <QDir>
#include <QFileInfo>

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    ~TerminalWidget();

private slots:
    void executeCommand();
    void readOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleSimulatedCommand(const QString &command);

private:
    void setupUI();
    void appendOutput(const QString &text);
    void handleKeyPress(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event) override;

    QTextEdit *outputArea;
    QLineEdit *commandInput;
    QLabel *statusLabel;
    QProcess *process;
    QStringList commandHistory;
    int historyIndex;
    QString currentWorkingDirectory;
    bool isSimulatedMode;
};

#endif // TERMINALWIDGET_H
