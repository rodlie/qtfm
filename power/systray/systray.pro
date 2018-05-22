QT += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtfm-power
VERSION = 1.0.0
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += systray.qrc
INCLUDEPATH += ..

CONFIG += link_pkgconfig
PKGCONFIG += x11 xscrnsaver
#LIBS += -lX11 -lXss

include(../../qtfm.pri)
DEFINES += CONFIG_DIR=\"\\\"$${QTFM_TARGET}$${QTFM_MAJOR}\\\"\"

exists(../../libpower) {
    message("Using embedded libpower")
    INCLUDEPATH += ../../libpower
    LIBS += -L../../libpower -lPower
} else {
    message("Using external libpower")
    CONFIG += link_pkgconfig
    PKGCONFIG += Power
}

target.path = $${PREFIX}/bin
target_desktop.path = $${XDGDIR}/autostart
target_desktop.files = $${TARGET}.desktop
target_docs.path = $${DOCDIR}/$${TARGET}-$${VERSION}
target_docs.files = ../../LICENSE ../../README.md
INSTALLS += target
# target_desktop target_docs
