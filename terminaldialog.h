#ifndef TERMINALDIALOG_H
#define TERMINALDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QKeyEvent>
#include <QFont>

class TouchInput;
class KeyboardDialog;

class TerminalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TerminalDialog(QWidget *parent = nullptr);
    ~TerminalDialog();

private slots:
    void executeCommand();
    void readOutput();
    void readError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void appendOutput(const QString &text);
    void clearOutput();

private:
    void handleKeyPress(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

    QTextEdit *outputArea;
    TouchInput *commandInput;
    QProcess *process;
    QStringList commandHistory;
    int historyIndex;
    QString currentWorkingDirectory;
};

#endif // TERMINALDIALOG_H
