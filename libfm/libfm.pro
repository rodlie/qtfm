QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtFM
VERSION = 1.0.0

TEMPLATE = lib
SOURCES += \
    desktopfile.cpp \
    fileutils.cpp \
    mimeutils.cpp \
    properties.cpp \
    progressdlg.cpp
HEADERS += \
    desktopfile.h \
    fileutils.h \
    mimeutils.h \
    properties.h \
    common.h \
    progressdlg.h
unix:!macx {
    !CONFIG(no_dbus) {
        HEADERS += service.h
        QT += dbus
    }
}

exists(../qtfm.pri) {
    include(../qtfm.pri)
}

lessThan(QT_MAJOR_VERSION, 5): LIBS += -lmagic
