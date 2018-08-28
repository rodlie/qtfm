#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
#

QT += core gui dbus widgets
include(../qtfm.pri)

TARGET = qtfm-tray
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += tray.qrc
OTHER_FILES += README.md

INCLUDEPATH += ../libfm
LIBS += -L../libfm -lQtFM

target.path = $${PREFIX}/bin
target_desktop.path = $${XDGDIR}/autostart
target_desktop.files = $${TARGET}.desktop
man.files += qtfm-tray.1
man.path += $${MANDIR}/man1
INSTALLS += target target_desktop man

CONFIG(staticlib) {
    include (../libfm/static.pri)
}
