QT += core dbus gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtFM
VERSION = 1.0.0

TEMPLATE = lib
SOURCES += desktopfile.cpp fileutils.cpp mimeutils.cpp properties.cpp progressdlg.cpp
HEADERS += desktopfile.h fileutils.h mimeutils.h properties.h common.h upower.h progressdlg.h \
    service.h

exists(../qtfm.pri) {
    include(../qtfm.pri)
}
