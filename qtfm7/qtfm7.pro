#-------------------------------------------------
#
# Project created by QtCreator 2018-08-24T02:51:49
#
#-------------------------------------------------

QT += core gui widgets
include(../qtfm.pri)
QTFM_MAJOR = 7
QTFM_MINOR = 0
QTFM_PATCH = 0

TARGET = qtfm7
TEMPLATE = app

#DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#CONFIG += c++11

SOURCES += \
        main.cpp \
        qtfm.cpp

HEADERS += \
        qtfm.h

INCLUDEPATH += ../libfm
LIBS += -L../libfm -lQtFM

