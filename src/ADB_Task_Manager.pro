#-------------------------------------------------
#
# Project created by QtCreator 2016-10-10T00:57:59
#
#-------------------------------------------------

QT       += core gui
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ADB_Task_Manager
TEMPLATE = app


QMAKE_CXXFLAGS += -std=c++1y
QMAKE_LFLAGS += -shared-libgcc

CONFIG += C++1y
QMAKE_LINK += -shared-libgcc

SOURCES += main.cpp\
        MainWindow.cpp \
    qcustomplot.cpp

HEADERS  += MainWindow.h \
    qcustomplot.h

FORMS    += MainWindow.ui

CONFIG += mobility
MOBILITY = 

