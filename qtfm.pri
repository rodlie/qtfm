QTFM_TARGET = qtfm
QTFM_TARGET_NAME = "QtFM"
QTFM_MAJOR = 6
QTFM_MINOR = 2
QTFM_PATCH = 0

QMAKE_TARGET_COMPANY = "QtFM"
QMAKE_TARGET_PRODUCT = "QtFM"
QMAKE_TARGET_DESCRIPTION = "QtFM"
QMAKE_TARGET_COPYRIGHT = "Copyright QtFM developers"

unix:!macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
        isEmpty(XDGDIR): XDGDIR = $${PREFIX}/etc/xdg
    }
    isEmpty(LIBDIR): LIBDIR = $$PREFIX/lib$${LIBSUFFIX}
    isEmpty(DOCDIR): DOCDIR = $$PREFIX/share/doc
    isEmpty(MANDIR): MANDIR = $$PREFIX/share/man
    isEmpty(XDGDIR): XDGDIR = /etc/xdg
}

CONFIG += link_pkgconfig

macx {
    QTFM_TARGET = QtFM
    LIBS += -framework CoreFoundation -framework CoreServices
    QT_CONFIG -= no-pkg-config
    PKGCONFIG += libinotify
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    CONFIG += staticlib
}

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
freebsd: LIBS += -linotify
netbsd-g++: PKGCONFIG += libinotify

lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")
