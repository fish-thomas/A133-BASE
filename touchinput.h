#ifndef TOUCHINPUT_H
#define TOUCHINPUT_H

#include <QLineEdit>
#include <QMouseEvent>

class TouchInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit TouchInput(const QString &contents = QString(), QWidget *parent = nullptr);
    ~TouchInput();

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void clicked();
};

#endif // TOUCHINPUT_H
