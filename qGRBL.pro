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
        mainwindow.cpp \
    CGRBLController.cpp \
    mainwindow_manualcontrol.cpp \
    grbl_settings.cpp

HEADERS  += mainwindow.h \
    CGRBLController.h \
    grbl_settings.h

FORMS    += mainwindow.ui \
    grbl_settings.ui

RESOURCES += \
    rsrc.qrc
