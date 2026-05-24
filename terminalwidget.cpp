#include "terminalwidget.h"
#include <QApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDir>
#include <QFileInfo>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
    , historyIndex(-1)
    , isSimulatedMode(true)
{
    currentWorkingDirectory = QDir::homePath();
    setupUI();
}

TerminalWidget::~TerminalWidget()
{
    if (process) {
        process->kill();
        process->deleteLater();
    }
}

void TerminalWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *titleLabel = new QLabel("HelperBoard A133 Terminal", this);
    titleLabel->setStyleSheet("background-color: #2c3e50; color: #ecf0f1; padding: 5px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    outputArea = new QTextEdit(this);
    outputArea->setReadOnly(true);
    outputArea->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #00ff00; font-family: 'Courier New', monospace; font-size: 12px; border: 1px solid #333; }");
    mainLayout->addWidget(outputArea);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    statusLabel = new QLabel("root@helperboard:~#", this);
    statusLabel->setStyleSheet("color: #3498db; font-family: 'Courier New', monospace; font-weight: bold;");
    inputLayout->addWidget(statusLabel);

    commandInput = new QLineEdit(this);
    commandInput->setStyleSheet("QLineEdit { background-color: #1e1e1e; color: #00ff00; font-family: 'Courier New', monospace; border: 1px solid #333; padding: 5px; }");
    commandInput->installEventFilter(this);
    connect(commandInput, &QLineEdit::returnPressed, this, &TerminalWidget::executeCommand);
    inputLayout->addWidget(commandInput);

    QPushButton *clearBtn = new QPushButton("Clear", this);
    clearBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; padding: 5px 10px; border: none; border-radius: 3px; }");
    connect(clearBtn, &QPushButton::clicked, [this]() {
        outputArea->clear();
    });
    inputLayout->addWidget(clearBtn);

    mainLayout->addLayout(inputLayout);

    appendOutput("========================================");
    appendOutput("  HelperBoard A133 Terminal Emulator");
    appendOutput("========================================");
    appendOutput("");
    appendOutput("Welcome! Type 'help' for available commands.");
    appendOutput("System started at: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    appendOutput("");
    commandInput->setFocus();

    process = nullptr;
}

void TerminalWidget::appendOutput(const QString &text)
{
    outputArea->append(text);
    outputArea->verticalScrollBar()->setValue(outputArea->verticalScrollBar()->maximum());
}

bool TerminalWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == commandInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        handleKeyPress(keyEvent);
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

void TerminalWidget::handleKeyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (!commandHistory.isEmpty() && historyIndex < commandHistory.size() - 1) {
            historyIndex++;
            commandInput->setText(commandHistory[commandHistory.size() - 1 - historyIndex]);
        }
        event->accept();
    } else if (event->key() == Qt::Key_Down) {
        if (historyIndex > 0) {
            historyIndex--;
            commandInput->setText(commandHistory[commandHistory.size() - 1 - historyIndex]);
        } else if (historyIndex == 0) {
            historyIndex = -1;
            commandInput->clear();
        }
        event->accept();
    }
}

void TerminalWidget::executeCommand()
{
    QString command = commandInput->text().trimmed();
    if (command.isEmpty()) {
        return;
    }

    QString prompt = QString("root@helperboard:%1# %2").arg(
        currentWorkingDirectory.replace(QDir::homePath(), "~"), command);
    appendOutput(prompt);

    commandHistory.append(command);
    historyIndex = -1;
    commandInput->clear();

    if (isSimulatedMode) {
        handleSimulatedCommand(command);
    } else {
        if (process) {
            process->kill();
            delete process;
        }

        process = new QProcess(this);
        connect(process, &QProcess::readyReadStandardOutput, this, &TerminalWidget::readOutput);
        connect(process, &QProcess::readyReadStandardError, this, &TerminalWidget::readOutput);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &TerminalWidget::processFinished);

        QStringList args = QProcess::splitCommand(command);
        if (!args.isEmpty()) {
            QString prog = args.takeFirst();
            process->start(prog, args);
        }
    }
}

void TerminalWidget::handleSimulatedCommand(const QString &command)
{
    QString cmd = command.toLower().trimmed();
    QStringList parts = command.trimmed().split(' ', Qt::SkipEmptyParts);
    QString baseCmd = parts.isEmpty() ? "" : parts[0].toLower();

    if (baseCmd == "help") {
        appendOutput("");
        appendOutput("Available commands:");
        appendOutput("  help      - Show this help message");
        appendOutput("  clear     - Clear the terminal screen");
        appendOutput("  cls       - Clear the terminal screen");
        appendOutput("  pwd       - Print working directory");
        appendOutput("  cd <dir>  - Change directory");
        appendOutput("  ls        - List directory contents");
        appendOutput("  date      - Show current date and time");
        appendOutput("  whoami    - Show current user");
        appendOutput("  hostname  - Show hostname");
        appendOutput("  uname     - Show system information");
        appendOutput("  ifconfig  - Show network interfaces");
        appendOutput("  top       - Show system processes (simulated)");
        appendOutput("  cat <file> - Display file contents (simulated)");
        appendOutput("  echo <msg> - Print message");
        appendOutput("  uptime    - Show system uptime");
        appendOutput("  free      - Show memory usage (simulated)");
        appendOutput("  df        - Show disk usage (simulated)");
        appendOutput("");
    }
    else if (baseCmd == "clear" || baseCmd == "cls") {
        outputArea->clear();
    }
    else if (baseCmd == "pwd") {
        appendOutput(currentWorkingDirectory);
    }
    else if (baseCmd == "cd") {
        if (parts.size() > 1) {
            QString path = parts[1];
            if (path == "~") {
                currentWorkingDirectory = QDir::homePath();
            } else if (path == "..") {
                QDir dir(currentWorkingDirectory);
                if (dir.cdUp()) {
                    currentWorkingDirectory = dir.path();
                }
            } else if (path.startsWith("/")) {
                QDir dir(path);
                if (dir.exists()) {
                    currentWorkingDirectory = path;
                } else {
                    appendOutput(QString("cd: %1: No such file or directory").arg(path));
                }
            } else {
                QDir dir(currentWorkingDirectory);
                if (dir.cd(path)) {
                    currentWorkingDirectory = dir.path();
                } else {
                    appendOutput(QString("cd: %1: No such file or directory").arg(path));
                }
            }
        } else {
            currentWorkingDirectory = QDir::homePath();
        }
        statusLabel->setText(QString("root@helperboard:%1#").arg(
            currentWorkingDirectory.replace(QDir::homePath(), "~")));
    }
    else if (baseCmd == "ls") {
        QDir dir(currentWorkingDirectory);
        QStringList entries;
        if (parts.contains("-la") || parts.contains("-al")) {
            entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                QString fullPath = currentWorkingDirectory + "/" + entry;
                QFileInfo info(fullPath);
                QString perms;
                perms += info.isDir() ? "d" : "-";
                perms += info.permission(QFile::ReadOwner) ? "r" : "-";
                perms += info.permission(QFile::WriteOwner) ? "w" : "-";
                perms += info.permission(QFile::ExeOwner) ? "x" : "-";
                perms += info.permission(QFile::ReadGroup) ? "r" : "-";
                perms += info.permission(QFile::WriteGroup) ? "w" : "-";
                perms += info.permission(QFile::ExeGroup) ? "x" : "-";
                perms += info.permission(QFile::ReadOther) ? "r" : "-";
                perms += info.permission(QFile::WriteOther) ? "w" : "-";
                perms += info.permission(QFile::ExeOther) ? "x" : "-";
                appendOutput(QString("%1  1 root root %2 %3")
                    .arg(perms)
                    .arg(info.size(), 8)
                    .arg(entry));
            }
        } else {
            entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
            appendOutput(entries.join("  "));
        }
    }
    else if (baseCmd == "date") {
        appendOutput(QDateTime::currentDateTime().toString("ddd MMM dd hh:mm:ss CST yyyy"));
    }
    else if (baseCmd == "whoami") {
        appendOutput("root");
    }
    else if (baseCmd == "hostname") {
        appendOutput("helperboard");
    }
    else if (baseCmd == "uname") {
        if (parts.size() > 1 && parts[1] == "-a") {
            appendOutput("Linux helperboard 5.10.0 #1 SMP PREEMPT arm64 aarch64 GNU/Linux");
        } else {
            appendOutput("Linux");
        }
    }
    else if (baseCmd == "ifconfig") {
        appendOutput("eth0      Link encap:Ethernet  HWaddr 00:11:22:33:44:55");
        appendOutput("          inet addr:192.168.1.100  Bcast:192.168.1.255  Mask:255.255.255.0");
        appendOutput("          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1");
        appendOutput("");
        appendOutput("lo        Link encap:Local Loopback");
        appendOutput("          inet addr:127.0.0.1  Mask:255.0.0.0");
        appendOutput("          UP LOOPBACK RUNNING  MTU:65536  Metric:1");
    }
    else if (baseCmd == "top") {
        appendOutput("top - " + QDateTime::currentDateTime().toString("hh:mm:ss") + " up 1 day,  1:30,  1 user,  load average: 0.52, 0.48, 0.51");
        appendOutput("Tasks:  85 total,   1 running,  84 sleeping,   0 stopped,   0 zombie");
        appendOutput("%Cpu(s):  5.2 us,  2.1 sy,  0.0 ni, 92.1 id,  0.0 wa,  0.0 hi,  0.6 si,  0.0 st");
        appendOutput("MiB Mem :  1982.4 total,   456.2 free,   823.1 used,   703.1 buff/cache");
        appendOutput("MiB Swap:   512.0 total,   512.0 free,    0.0 used.   985.2 avail Mem");
        appendOutput("");
        appendOutput("  PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND");
        appendOutput("    1 root      20   0    8320    3560    2108 S   0.0   0.2   0:02.34 init");
        appendOutput("  234 root      20   0   12500    4560    3100 S   0.0   0.2   0:01.23 systemd");
        appendOutput("  567 root      20   0  156000   12000    8900 S   0.0   0.6   0:05.67 rgb_display_demo");
    }
    else if (baseCmd == "uptime") {
        appendOutput(" " + QDateTime::currentDateTime().toString("hh:mm:ss") + " up 1 day,  1:30,  1 user,  load average: 0.52, 0.48, 0.51");
    }
    else if (baseCmd == "free") {
        appendOutput("              total        used        free      shared  buff/cache   available");
        appendOutput("Mem:        2030736      842560      467232       10240      720944      985632");
        appendOutput("Swap:        524284           0      524284");
    }
    else if (baseCmd == "df") {
        appendOutput("Filesystem      1K-blocks    Used Available Use% Mounted on");
        appendOutput("/dev/root        31457280  5678900  25778380  18% /");
        appendOutput("tmpfs             1000000       100     999900   1% /run");
        appendOutput("/dev/mmcblk0p1     204800    32768    172032  16% /boot");
    }
    else if (baseCmd == "echo") {
        QString msg = parts.size() > 1 ? parts.mid(1).join(" ") : "";
        appendOutput(msg);
    }
    else if (baseCmd == "cat") {
        if (parts.size() > 1) {
            appendOutput(QString("[Simulated] Contents of %1:").arg(parts[1]));
            appendOutput("This is simulated file content.");
            appendOutput("In real mode, this would show actual file contents.");
        } else {
            appendOutput("cat: missing operand");
        }
    }
    else {
        appendOutput(QString("%1: command not found").arg(baseCmd));
        appendOutput("Type 'help' to see available commands.");
    }

    appendOutput("");
}

void TerminalWidget::readOutput()
{
    if (process) {
        QString output = QString::fromLocal8Bit(process->readAll());
        appendOutput(output);
    }
}

void TerminalWidget::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        appendOutput(QString("Process exited with code: %1").arg(exitCode));
    } else {
        appendOutput("Process terminated abnormally");
    }
    appendOutput("");
}
