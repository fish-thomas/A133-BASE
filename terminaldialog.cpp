#include "terminaldialog.h"
#include "touchinput.h"
#include "keyboarddialog.h"
#include <QDebug>
#include <QScrollBar>
#include <QSizePolicy>
#include <QDir>
#include <QWidget>
#include <QLayout>

TerminalDialog::TerminalDialog(QWidget *parent)
    : QDialog(parent)
    , process(nullptr)
    , historyIndex(-1)
    , outputArea(nullptr)
    , commandInput(nullptr)
{
    setWindowTitle("Terminal");
    resize(760, 360);
    setMaximumSize(760, 360);
    setStyleSheet("background-color: #1e1e1e;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(2);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(0);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    outputArea = new QTextEdit(this);
    outputArea->setReadOnly(true);
    outputArea->setStyleSheet(R"(
        QTextEdit {
            background-color: #1e1e1e;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            padding: 2px;
            border: 1px solid #333;
            border-radius: 2px;
        }
    )");
    outputArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLayout->addWidget(outputArea, 10);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(4);
    inputLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *promptLabel = new QLabel("$", this);
    promptLabel->setStyleSheet("color: #00ff00; font-family: 'Courier New', monospace; font-size: 16px; font-weight: bold;");
    promptLabel->setFixedWidth(20);
    inputLayout->addWidget(promptLabel);

    commandInput = new TouchInput(QString(), this);
    commandInput->setPlaceholderText("Enter command");
    commandInput->setStyleSheet(R"(
        TouchInput {
            background-color: #2d2d2d;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            padding: 2px;
            border: 1px solid #444;
            border-radius: 2px;
        }
        TouchInput:focus {
            border: 1px solid #00ff00;
        }
    )");
    commandInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    commandInput->setMaximumHeight(40);
    connect(commandInput, &TouchInput::clicked, [this]() {
        KeyboardDialog dialog("Terminal", this);
        dialog.setInputText(commandInput->text());
        if (dialog.exec() == QDialog::Accepted) {
            commandInput->setText(dialog.getInputText());
        }
    });
    connect(commandInput, &QLineEdit::returnPressed, this, &TerminalDialog::executeCommand);
    inputLayout->addWidget(commandInput, 1);

    QPushButton *executeButton = new QPushButton("Execute", this);
    executeButton->setMinimumSize(80, 35);
    executeButton->setMaximumHeight(40);
    executeButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 4px; border: none; border-radius: 4px; font-size: 13px; font-weight: bold; }");
    connect(executeButton, &QPushButton::clicked, this, &TerminalDialog::executeCommand);
    inputLayout->addWidget(executeButton);

    leftLayout->addLayout(inputLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(1);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *clearButton = new QPushButton("Clear", this);
    clearButton->setMinimumSize(90, 45);
    clearButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 4px; border: none; border-radius: 4px; font-size: 13px; font-weight: bold; }");
    clearButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(clearButton, &QPushButton::clicked, this, &TerminalDialog::clearOutput);
    rightLayout->addWidget(clearButton, 1);

    QPushButton *closeButton = new QPushButton("Close", this);
    closeButton->setMinimumSize(90, 45);
    closeButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 4px; border: none; border-radius: 4px; font-size: 13px; font-weight: bold; }");
    closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    rightLayout->addWidget(closeButton, 1);

    contentLayout->addLayout(leftLayout, 4);
    contentLayout->addLayout(rightLayout, 1);

    mainLayout->addLayout(contentLayout, 1);

    appendOutput("Terminal Ready\n");
    appendOutput("Type command and press Execute\n\n");

    currentWorkingDirectory = QDir::homePath();
}

TerminalDialog::~TerminalDialog()
{
    if (process) {
        process->kill();
        process->deleteLater();
    }
}

void TerminalDialog::executeCommand()
{
    QString command = commandInput->text().trimmed();
    if (command.isEmpty()) {
        return;
    }

    commandHistory.append(command);
    historyIndex = commandHistory.size();

    appendOutput(QString("$ %1\n").arg(command));

    commandInput->clear();

    if (process) {
        process->kill();
        process->deleteLater();
    }

    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, &TerminalDialog::readOutput);
    connect(process, &QProcess::readyReadStandardError, this, &TerminalDialog::readError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalDialog::processFinished);

    QStringList args = QProcess::splitCommand(command);
    if (args.isEmpty()) {
        return;
    }

    QString program = args.takeFirst();
    process->start(program, args);
}

void TerminalDialog::readOutput()
{
    if (process) {
        QString output = process->readAllStandardOutput();
        appendOutput(output);
    }
}

void TerminalDialog::readError()
{
    if (process) {
        QString error = process->readAllStandardError();
        appendOutput(error);
    }
}

void TerminalDialog::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    if (exitCode != 0) {
        appendOutput(QString("\n[Exit code: %1]\n").arg(exitCode));
    }
    appendOutput("\n");
}

void TerminalDialog::appendOutput(const QString &text)
{
    if (outputArea) {
        outputArea->moveCursor(QTextCursor::End);
        outputArea->insertPlainText(text);
        outputArea->moveCursor(QTextCursor::End);
        outputArea->verticalScrollBar()->setValue(outputArea->verticalScrollBar()->maximum());
    }
}

void TerminalDialog::clearOutput()
{
    if (outputArea) {
        outputArea->clear();
        appendOutput("Terminal Ready\n\n");
    }
}

void TerminalDialog::handleKeyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (!commandHistory.isEmpty() && historyIndex > 0) {
            historyIndex--;
            commandInput->setText(commandHistory.at(historyIndex));
        }
        event->accept();
    } else if (event->key() == Qt::Key_Down) {
        if (!commandHistory.isEmpty() && historyIndex < commandHistory.size() - 1) {
            historyIndex++;
            commandInput->setText(commandHistory.at(historyIndex));
        } else {
            historyIndex = commandHistory.size();
            commandInput->clear();
        }
        event->accept();
    }
}

bool TerminalDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == commandInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        handleKeyPress(keyEvent);
    }
    return QDialog::eventFilter(obj, event);
}
