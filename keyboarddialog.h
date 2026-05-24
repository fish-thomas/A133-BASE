#ifndef KEYBOARDDIALOG_H
#define KEYBOARDDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <vector>

class KeyboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyboardDialog(const QString &title = "Keyboard", QWidget *parent = nullptr);
    ~KeyboardDialog();

    QString getText() const { return m_inputText; }
    QString getInputText() const { return m_inputText; }
    void setInputText(const QString &text) { m_inputText = text; if (m_inputDisplay) m_inputDisplay->setText(text); }
    static QString getText(const QString &title, const QString &defaultText = "", QWidget *parent = nullptr);

private:
    QString m_inputText;
    QLineEdit *m_inputDisplay;
    std::vector<QPushButton*> m_letterButtons;
    bool m_isShifted;
    bool m_isNumberMode;

    void setupUI();
    void createKeyRow(QVBoxLayout *mainLayout, const std::vector<QString> &keys);
    void createNumberRow(QVBoxLayout *mainLayout);
    void createQWERTYRow(QVBoxLayout *mainLayout);
    void createASDFRow(QVBoxLayout *mainLayout);
    void createZXCVRow(QVBoxLayout *mainLayout);
    void createFunctionRow(QVBoxLayout *mainLayout);
    void createNumberKeys(QVBoxLayout *mainLayout);
    void updateKeyboardMode();

private slots:
    void onKeyPressed(const QString &key);
    void onShiftPressed();
    void onBackspacePressed();
    void onSpacePressed();
    void onEnterPressed();
    void onClearPressed();
    void onClosePressed();
    void onToggleNumbers();

signals:
    void textEntered(const QString &text);
};

#endif // KEYBOARDDIALOG_H
