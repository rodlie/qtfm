QT += core dbus gui widgets

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
    progressdlg.h \
    delegates.h \
    sortfilter.h
!CONFIG(no_dbus) {
    HEADERS += service.h
    QT += dbus
}

exists(../qtfm.pri) {
    include(../qtfm.pri)
}
