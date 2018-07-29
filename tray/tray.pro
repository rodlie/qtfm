#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
#

QT += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(../qtfm.pri)

TARGET = qtfm-tray
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += tray.qrc
OTHER_FILES += README.md

exists(../libdisks/libdisks.pro) {
    INCLUDEPATH += ../libdisks
    LIBS += -L../libdisks -lDisks
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += Disks
}

INCLUDEPATH += ../libfm
LIBS += -L../libfm -lQtFM

target.path = $${PREFIX}/bin
target_desktop.path = $${XDGDIR}/autostart
target_desktop.files = $${TARGET}.desktop
man.files += qtfm-tray.8
man.path += $${MANDIR}/man8
INSTALLS += target target_desktop man

lessThan(QT_MAJOR_VERSION, 5): LIBS += -lmagic
