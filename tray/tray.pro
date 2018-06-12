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

exists(../libdisks/libdisks.pro) {
    message("Using embedded libdisks")
    INCLUDEPATH += ../libdisks
    LIBS += -L../libdisks -lDisks
} else {
    message("Using external libdisks")
    CONFIG += link_pkgconfig
    PKGCONFIG += Disks
}

target.path = $${PREFIX}/bin
target_desktop.path = $${XDGDIR}/autostart
target_desktop.files = $${TARGET}.desktop
INSTALLS += target target_desktop
