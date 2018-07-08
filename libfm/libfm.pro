QT += core dbus gui
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
!CONFIG(no_dbus) {
    HEADERS += upower.h service.h
    QT += dbus
}

exists(../qtfm.pri) {
    include(../qtfm.pri)
}
