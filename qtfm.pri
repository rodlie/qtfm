isEmpty(PREFIX) {
    PREFIX = /usr/local
    isEmpty(XDGDIR) {
        XDGDIR = $${PREFIX}/etc/xdg
    }
}
isEmpty(DOCDIR) {
    DOCDIR = $$PREFIX/share/doc
}
isEmpty(XDGDIR) {
    XDGDIR = /etc/xdg
}
CONFIG += staticlib
CONFIG -= install_lib
