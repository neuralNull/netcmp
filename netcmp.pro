#-------------------------------------------------
#
# Project created by QtCreator 2017-07-04T09:39:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = netcmp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    netlist.cpp \
    netcmpmodel.cpp

HEADERS  += mainwindow.h \
    netlist.h \
    netcmpmodel.h

FORMS    += mainwindow.ui

RC_FILE += netcmp.rc
