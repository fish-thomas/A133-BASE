#include "keyboarddialog.h"
#include <QKeyEvent>
#include <QFont>

KeyboardDialog::KeyboardDialog(const QString &title, QWidget *parent)
    : QDialog(parent)
    , m_inputText("")
    , m_inputDisplay(nullptr)
    , m_isShifted(false)
    , m_isNumberMode(false)
{
    setWindowTitle(title);
    setWindowFlags(windowFlags() | Qt::Dialog | Qt::FramelessWindowHint);
    setupUI();
}

KeyboardDialog::~KeyboardDialog()
{
}

void KeyboardDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    QLabel *titleLabel = new QLabel(windowTitle());
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; padding: 4px; background: #333; color: white; border-radius: 3px; }");
    mainLayout->addWidget(titleLabel);

    m_inputDisplay = new QLineEdit();
    m_inputDisplay->setAlignment(Qt::AlignLeft);
    m_inputDisplay->setReadOnly(true);
    m_inputDisplay->setText(m_inputText);
    m_inputDisplay->setStyleSheet("QLineEdit { padding: 6px; font-size: 14px; font-family: 'Courier New'; background: #1e1e1e; color: #00ff00; border: 2px solid #444; border-radius: 3px; }");
    mainLayout->addWidget(m_inputDisplay);

    QPushButton *closeButton = new QPushButton("Close");
    closeButton->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #f44336; color: white; border: none; border-radius: 4px; }");
    connect(closeButton, &QPushButton::clicked, this, &KeyboardDialog::onClosePressed);
    mainLayout->addWidget(closeButton);

    createNumberKeys(mainLayout);

    setLayout(mainLayout);
    resize(780, 460);
    setMaximumSize(780, 460);
}

void KeyboardDialog::createNumberKeys(QVBoxLayout *mainLayout)
{
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(3);

    QPushButton *btn = new QPushButton("⌫");
    btn->setMinimumSize(50, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #555; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onBackspacePressed);
    row1->addWidget(btn);

    QString numKeys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
    for (int i = 0; i < 10; i++) {
        btn = new QPushButton(numKeys[i]);
        btn->setMinimumSize(45, 40);
        btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
        QString key = numKeys[i];
        connect(btn, &QPushButton::clicked, this, [this, key]() { onKeyPressed(key); });
        row1->addWidget(btn);
    }

    btn = new QPushButton("Close");
    btn->setMinimumSize(55, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #f44336; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onClosePressed);
    row1->addWidget(btn);

    mainLayout->addLayout(row1);

    QString row2Chars[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
    createKeyRow(mainLayout, std::vector<QString>(row2Chars, row2Chars + 10));

    QString row3Chars[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l"};
    createKeyRow(mainLayout, std::vector<QString>(row3Chars, row3Chars + 9));

    QHBoxLayout *row4 = new QHBoxLayout();
    row4->setSpacing(3);

    btn = new QPushButton("⇧");
    btn->setMinimumSize(50, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #2196F3; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onShiftPressed);
    row4->addWidget(btn);

    QString row4Chars[] = {"z", "x", "c", "v", "b", "n", "m"};
    for (int i = 0; i < 7; i++) {
        btn = new QPushButton(row4Chars[i]);
        btn->setMinimumSize(45, 40);
        btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
        QString key = row4Chars[i];
        connect(btn, &QPushButton::clicked, this, [this, key]() { onKeyPressed(key); });
        m_letterButtons.push_back(btn);
        row4->addWidget(btn);
    }

    btn = new QPushButton("-");
    btn->setMinimumSize(45, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, [this]() { onKeyPressed("-"); });
    row4->addWidget(btn);

    btn = new QPushButton("_");
    btn->setMinimumSize(45, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, [this]() { onKeyPressed("_"); });
    row4->addWidget(btn);

    mainLayout->addLayout(row4);

    QHBoxLayout *row5 = new QHBoxLayout();
    row5->setSpacing(3);

    btn = new QPushButton("123");
    btn->setMinimumSize(55, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #9C27B0; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onToggleNumbers);
    row5->addWidget(btn);

    btn = new QPushButton(":");
    btn->setMinimumSize(45, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, [this]() { onKeyPressed(":"); });
    row5->addWidget(btn);

    btn = new QPushButton("/");
    btn->setMinimumSize(45, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, [this]() { onKeyPressed("/"); });
    row5->addWidget(btn);

    btn = new QPushButton("Space");
    btn->setMinimumSize(180, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onSpacePressed);
    row5->addWidget(btn);

    btn = new QPushButton(".");
    btn->setMinimumSize(45, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, [this]() { onKeyPressed("."); });
    row5->addWidget(btn);

    btn = new QPushButton("Clear");
    btn->setMinimumSize(55, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #FF9800; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onClearPressed);
    row5->addWidget(btn);

    btn = new QPushButton("Enter");
    btn->setMinimumSize(65, 40);
    btn->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; background: #4CAF50; color: white; border: none; border-radius: 4px; }");
    connect(btn, &QPushButton::clicked, this, &KeyboardDialog::onEnterPressed);
    row5->addWidget(btn);

    mainLayout->addLayout(row5);
}

void KeyboardDialog::createKeyRow(QVBoxLayout *mainLayout, const std::vector<QString> &keys)
{
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(3);

    for (size_t i = 0; i < keys.size(); i++) {
        QPushButton *btn = new QPushButton(keys[i]);
        btn->setMinimumSize(45, 40);
        btn->setStyleSheet("QPushButton { padding: 6px; font-size: 14px; background: #4a4a4a; color: white; border: none; border-radius: 4px; }");
        QString key = keys[i];
        connect(btn, &QPushButton::clicked, this, [this, key]() { onKeyPressed(key); });
        m_letterButtons.push_back(btn);
        row->addWidget(btn);
    }

    mainLayout->addLayout(row);
}

void KeyboardDialog::onKeyPressed(const QString &key)
{
    QString outputKey = key;
    if (m_isShifted && key.length() == 1 && key[0].isLetter()) {
        outputKey = key.toUpper();
        m_isShifted = false;
        updateKeyboardMode();
    }
    m_inputText += outputKey;
    m_inputDisplay->setText(m_inputText);
}

void KeyboardDialog::onShiftPressed()
{
    m_isShifted = !m_isShifted;
    updateKeyboardMode();
}

void KeyboardDialog::updateKeyboardMode()
{
    for (size_t i = 0; i < m_letterButtons.size(); i++) {
        QString text = m_letterButtons[i]->text();
        if (m_isShifted && text.length() == 1) {
            m_letterButtons[i]->setText(text.toUpper());
        } else if (!m_isShifted && text.length() == 1) {
            m_letterButtons[i]->setText(text.toLower());
        }
    }
}

void KeyboardDialog::onBackspacePressed()
{
    if (!m_inputText.isEmpty()) {
        m_inputText.chop(1);
        m_inputDisplay->setText(m_inputText);
    }
}

void KeyboardDialog::onSpacePressed()
{
    m_inputText += " ";
    m_inputDisplay->setText(m_inputText);
}

void KeyboardDialog::onEnterPressed()
{
    accept();
}

void KeyboardDialog::onClearPressed()
{
    m_inputText.clear();
    m_inputDisplay->clear();
}

void KeyboardDialog::onClosePressed()
{
    reject();
}

void KeyboardDialog::onToggleNumbers()
{
    m_isShifted = !m_isShifted;
    updateKeyboardMode();
}

QString KeyboardDialog::getText(const QString &title, const QString &defaultText, QWidget *parent)
{
    KeyboardDialog dialog(title, parent);
    dialog.m_inputText = defaultText;
    dialog.m_inputDisplay->setText(defaultText);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.m_inputText;
    }
    return QString();
}
