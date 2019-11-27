#-------------------------------------------------
#
# Project created by QtCreator 2019-01-17T20:38:52
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hw6_1
TEMPLATE = app
CONFIG += c++14


SOURCES += main.cpp\
        mainwindow.cpp \
    finalwindow.cpp \
    filesdialog.cpp

HEADERS  += mainwindow.h \
    finalwindow.h \
    filesdialog.h

FORMS    += mainwindow.ui \
    finalwindow.ui \
    filesdialog.ui
