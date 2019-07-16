QTFM_TARGET = qtfm
QTFM_TARGET_NAME = "QtFM"
QTFM_MAJOR = 6
QTFM_MINOR = 2
QTFM_PATCH = 1

QMAKE_TARGET_COMPANY = "$${QTFM_TARGET_NAME}"
QMAKE_TARGET_PRODUCT = "$${QTFM_TARGET_NAME}"
QMAKE_TARGET_DESCRIPTION = "$${QTFM_TARGET_NAME}"
QMAKE_TARGET_COPYRIGHT = "Copyright $${QTFM_TARGET_NAME} developers"

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

QT += widgets

CONFIG += link_pkgconfig

macx {
    QTFM_TARGET = QtFM
    LIBS += -framework CoreFoundation -framework CoreServices
    QT_CONFIG -= no-pkg-config
    PKGCONFIG += libinotify
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    CONFIG += staticlib
}

CONFIG(deploy) : DEFINES += DEPLOY
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    !CONFIG(sharedlib): CONFIG += staticlib
}

freebsd: LIBS += -linotify
netbsd-g++: PKGCONFIG += libinotify

#DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")
