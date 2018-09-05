#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
#

QT += core gui widgets
include(../../qtfm.pri)

TARGET = qtfm-launcher
TEMPLATE = app

SOURCES += main.cpp dialog.cpp
HEADERS += dialog.h
RESOURCES += ../../qtfm.qrc

INCLUDEPATH += ../../libfm
LIBS += -L../../libfm -lQtFM

target.path = $${PREFIX}/bin
INSTALLS += target

CONFIG(staticlib) {
    include (../../libfm/static.pri)
}
