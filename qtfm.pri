QTFM_TARGET = qtfm
QTFM_TARGET_NAME = "QtFM"
QTFM_MAJOR = 6
QTFM_MINOR = 2
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

CONFIG += link_pkgconfig
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    !CONFIG(no_static) {
        CONFIG += staticlib
        CONFIG += no_install_lib
    }
}
