QT += core dbus gui widgets

TARGET = QtFM
VERSION = 1.2.0

CONFIG += install_lib

TEMPLATE = lib
SOURCES += \
    desktopfile.cpp \
    fileutils.cpp \
    mimeutils.cpp \
    properties.cpp \
    progressdlg.cpp \
    mymodelitem.cpp \
    mymodel.cpp \
    fm.cpp \
    disks.cpp \
    common.cpp \
    delegates.cpp \
    udisks2.cpp \
    sortfilter.cpp \
    service.cpp
HEADERS += \
    desktopfile.h \
    fileutils.h \
    mimeutils.h \
    properties.h \
    common.h \
    progressdlg.h \
    mymodelitem.h \
    mymodel.h \
    delegates.h \
    sortfilter.h \
    fm.h \
    disks.h \
    udisks2.h \
    service.h

exists(../qtfm.pri) {
    include(../qtfm.pri)
}

CONFIG(install_lib) {
    CONFIG += create_prl no_install_prl create_pc
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(DOCDIR) {
        DOCDIR = $$PREFIX/share/doc
    }

    target.path = $${PREFIX}/lib$${LIBSUFFIX}
    target_inc.path = $${PREFIX}/include/qtfm
    target_inc.files = $${HEADERS}

    QMAKE_PKGCONFIG_NAME = $${TARGET}
    QMAKE_PKGCONFIG_DESCRIPTION = QtFM Library
    QMAKE_PKGCONFIG_LIBDIR = $$target.path
    QMAKE_PKGCONFIG_INCDIR = $$target_inc.path
    QMAKE_PKGCONFIG_DESTDIR = pkgconfig

    INSTALLS += target target_inc
}
