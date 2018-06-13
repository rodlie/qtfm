#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
#

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(../qtfm.pri)

TARGET = qtfm-open
TEMPLATE = app

SOURCES += main.cpp dialog.cpp ../libfm/fileutils.cpp ../libfm/desktopfile.cpp ../fm/src/properties.cpp
HEADERS += dialog.h ../libfm/fileutils.h ../libfm/desktopfile.h ../fm/src/properties.h
INCLUDEPATH += ../libfm ../fm/src

target.path = $${PREFIX}/bin
INSTALLS += target
