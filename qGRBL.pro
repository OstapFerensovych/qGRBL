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
    mainwindow_manualcontrol.cpp

HEADERS  += mainwindow.h \
    CGRBLController.h

FORMS    += mainwindow.ui

RESOURCES += \
    rsrc.qrc
