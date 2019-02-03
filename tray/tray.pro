#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
#

QT += dbus widgets
include(../qtfm.pri)

TARGET = qtfm-tray
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += tray.qrc
OTHER_FILES += README.md

INCLUDEPATH += $${top_srcdir}/libfm
LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM

DESTDIR = $${top_builddir}/bin
OBJECTS_DIR = $${DESTDIR}/.obj_tray
MOC_DIR = $${DESTDIR}/.moc_tray
RCC_DIR = $${DESTDIR}/.qrc_tray

target.path = $${PREFIX}/bin
desktop.path = $${XDGDIR}/autostart
desktop.files = $${TARGET}.desktop
man.files += qtfm-tray.1
man.path += $${MANDIR}/man1
INSTALLS += target desktop man

!CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
