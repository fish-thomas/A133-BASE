QT += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = A133_BASE
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    wifidialog.cpp \
    terminalwidget.cpp \
    terminaldialog.cpp \
    keyboarddialog.cpp \
    touchinput.cpp \
    ethdialog.cpp

HEADERS += \
    mainwindow.h \
    wifidialog.h \
    terminalwidget.h \
    terminaldialog.h \
    keyboarddialog.h \
    touchinput.h \
    ethdialog.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
