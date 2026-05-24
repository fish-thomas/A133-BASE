#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("RGB Display Demo");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("HelperBoard");

    qDebug() << "HelperBoard A133 RGB Display Demo starting...";
    qDebug() << "Screen resolution:" << QApplication::desktop()->screenGeometry().width()
             << "x" << QApplication::desktop()->screenGeometry().height();

    MainWindow w;
    w.show();

    return app.exec();
}
