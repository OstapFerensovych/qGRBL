#-------------------------------------------------
#
# Project created by QtCreator 2015-02-28T12:17:57
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = grbl_comm
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    CGRBLController.cpp \
    MainWindow_ManualCtrl.cpp \
    SettingsWindow.cpp

HEADERS  += MainWindow.h \
    CGRBLController.h \
    SettingsWindow.h

FORMS    += MainWindow.ui \
    SettingsWindow.ui

RESOURCES += \
    rsrc.qrc
