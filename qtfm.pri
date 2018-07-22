QTFM_TARGET = qtfm
QTFM_TARGET_NAME = "QtFM"
QTFM_MAJOR = 6
QTFM_MINOR = 1
QTFM_PATCH = 0

isEmpty(PREFIX) {
    PREFIX = /usr/local
    isEmpty(XDGDIR) {
        XDGDIR = $${PREFIX}/etc/xdg
    }
}
isEmpty(DOCDIR) {
    DOCDIR = $$PREFIX/share/doc
}
isEmpty(MANDIR) {
    MANDIR = $$PREFIX/share/man
}
isEmpty(XDGDIR) {
    XDGDIR = /etc/xdg
}
CONFIG += staticlib
CONFIG -= install_lib

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
freebsd: LIBS += -linotify
