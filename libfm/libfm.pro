QT += core dbus gui widgets
include(../qtfm.pri)

TARGET = QtFM
VERSION = 1.$${QTFM_MINOR}.$${QTFM_PATCH}

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
    service.cpp \
    thumbs.cpp \
    customactionsmanager.cpp \
    processdialog.cpp \
    applicationdialog.cpp \
    settingsdialog.cpp \
    icondlg.cpp \
    propertiesdlg.cpp
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
    service.h \
    thumbs.h \
    customactionsmanager.h \
    processdialog.h \
    applicationdialog.h \
    settingsdialog.h \
    icondlg.h \
    propertiesdlg.h

CONFIG(magick7): DEFINES += MAGICK7
isEmpty(MAGICK_PKGCONFIG) {
    PKGCONFIG += Magick++
} else {
    PKGCONFIG += $${MAGICK_PKGCONFIG}
}
unix:!linux { LIBS += -linotify }
PKGCONFIG += libavdevice libswscale

!CONFIG(no_install_lib) {
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
