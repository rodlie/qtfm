QTFM_TARGET = qtfm
QTFM_TARGET_NAME = "QtFM"
QTFM_MAJOR = 6
QTFM_MINOR = 1
QTFM_PATCH = 6

QMAKE_TARGET_COMPANY = "QtFM"
QMAKE_TARGET_PRODUCT = "QtFM"
QMAKE_TARGET_DESCRIPTION = "QtFM"
QMAKE_TARGET_COPYRIGHT = "Copyright QtFM developers"

unix:!macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
        isEmpty(XDGDIR) {
            XDGDIR = $${PREFIX}/etc/xdg
        }
    }
    isEmpty(DOCDIR): DOCDIR = $$PREFIX/share/doc
    isEmpty(MANDIR): MANDIR = $$PREFIX/share/man
    isEmpty(XDGDIR): XDGDIR = /etc/xdg
}

CONFIG += staticlib
CONFIG -= install_lib

macx {
    QTFM_TARGET = QtFM
    LIBS += -framework CoreFoundation -framework CoreServices
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += libinotify
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
}

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
freebsd: LIBS += -linotify
