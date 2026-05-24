#include "touchinput.h"
#include "keyboarddialog.h"

TouchInput::TouchInput(const QString &contents, QWidget *parent)
    : QLineEdit(contents, parent)
{
    setReadOnly(true);
    setStyleSheet("QLineEdit { padding: 6px; font-size: 12px; background: white; border: 2px solid #ccc; border-radius: 5px; }");
}

TouchInput::~TouchInput()
{
}

void TouchInput::mousePressEvent(QMouseEvent *event)
{
    QLineEdit::mousePressEvent(event);
    emit clicked();

    QString result = KeyboardDialog::getText("Keyboard Input", text(), parentWidget());

    if (!result.isNull()) {
        setText(result);
    }
}
