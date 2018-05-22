QT += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtfm-power-settings
VERSION = 1.0.0
TEMPLATE = app

SOURCES += main.cpp dialog.cpp
HEADERS += dialog.h ../common.h
INCLUDEPATH += ..

include(../../qtfm.pri)

DEFINES += CONFIG_DIR=\"\\\"$${QTFM_TARGET}$${QTFM_MAJOR}\\\"\"

target.path = $${PREFIX}/bin
target_desktop.path = $${PREFIX}/share/applications
target_desktop.files = $${TARGET}.desktop
target_docs.path = $${DOCDIR}/$${TARGET}-$${VERSION}
target_docs.files = ../../LICENSE ../../README.md
INSTALLS += target target_desktop
# target_docs

RESOURCES += \
    res.qrc
