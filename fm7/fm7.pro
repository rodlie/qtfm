QT += widgets
include($${top_srcdir}/share/qtfm.pri)

QTFM_MAJOR = 7
QTFM_MINOR = 0
QTFM_PATCH = 0

TARGET = $${QTFM_TARGET}7
TARGET_NAME = $${QTFM_TARGET_NAME}
VERSION = $${QTFM_MAJOR}.$${QTFM_MINOR}.$${QTFM_PATCH}
TEMPLATE = app

INCLUDEPATH += $${top_srcdir}/libfm

DEFINES += APP=\"\\\"$${TARGET}\\\"\"
DEFINES += APP_NAME=\"\\\"$${TARGET_NAME}\\\"\"
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"

SOURCES += \
        main.cpp \
        qtfm.cpp

HEADERS += \
        qtfm.h

macx {
    LIBS += -L$${top_builddir}/libfm -lQtFM
    DEFINES += NO_DBUS NO_UDISKS
}
unix:!macx {
    DESTDIR = $${top_builddir}/bin
    OBJECTS_DIR = $${DESTDIR}/.obj_fm7
    MOC_DIR = $${DESTDIR}/.moc_fm7
    RCC_DIR = $${DESTDIR}/.qrc_fm7
    LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM
    CONFIG(no_dbus) {
        DEFINES += NO_DBUS
        DEFINES += NO_UDISKS
    }
    !CONFIG(no_dbus) : QT += dbus
    !CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
}


PKGCONFIG += qtermwidget5
