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
CONFIG += staticlib
CONFIG -= install_lib
CONFIG += link_pkgconfig

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
unix:!linux { LIBS += -linotify }

CONFIG(no_magick): DEFINES += NO_MAGICK
!CONFIG(no_magick) {
    isEmpty(MAGICK_PKGCONFIG) {
        PKGCONFIG += Magick++
    } else {
        PKGCONFIG += $${MAGICK_PKGCONFIG}
    }
}
CONFIG(legacy_magick): DEFINES += LEGACY_MAGICK
CONFIG(magick7): DEFINES += MAGICK7
