QT += widgets

TARGET = QtFM
VERSION = 1.2.0
TEMPLATE = lib

SOURCES += \
    applicationdialog.cpp \
    customactionsmanager.cpp \
    desktopfile.cpp \
    fileutils.cpp \
    mimeutils.cpp \
    properties.cpp \
    processdialog.cpp \
    progressdlg.cpp \
    icondlg.cpp \
    mymodel.cpp \
    mymodelitem.cpp \
    propertiesdlg.cpp \
    common.cpp \
    completer.cpp \
    sortmodel.cpp \
    iconview.cpp \
    iconlist.cpp

HEADERS += \
    applicationdialog.h \
    customactionsmanager.h \
    desktopfile.h \
    fileutils.h \
    mimeutils.h \
    properties.h \
    common.h \
    processdialog.h \
    progressdlg.h \
    icondlg.h \
    mymodel.h \
    mymodelitem.h \
    propertiesdlg.h \
    iconview.h \
    iconlist.h \
    completer.h \
    sortmodel.h

include(../qtfm.pri)

macx: DESTDIR = $${top_builddir}/libfm
unix:!macx {
    DESTDIR = $${top_builddir}/lib$${LIBSUFFIX}
    !CONFIG(no_dbus) {
        SOURCES += disks.cpp udisks2.cpp
        HEADERS += disks.h udisks2.h service.h
        QT += dbus
    }
    CONFIG(with_includes): CONFIG += create_prl no_install_prl create_pc
    target.path = $${LIBDIR}
    target_docs.path = $${DOCDIR}/qtfm-$${QTFM_MAJOR}.$${QTFM_MINOR}.$${QTFM_PATCH}
    target_docs.files = ../LICENSE ../README.md ../AUTHORS ../ChangeLog
    CONFIG(with_includes) {
        target_inc.path = $${PREFIX}/include/lib$${TARGET}
        target_inc.files = $${HEADERS}
        QMAKE_PKGCONFIG_NAME = lib$${TARGET}
        QMAKE_PKGCONFIG_DESCRIPTION = $${TARGET} library
        QMAKE_PKGCONFIG_LIBDIR = $$target.path
        QMAKE_PKGCONFIG_INCDIR = $$target_inc.path
        QMAKE_PKGCONFIG_DESTDIR = pkgconfig
    }
    INSTALLS += target target_docs
    CONFIG(with_includes): INSTALLS += target_inc
}

OBJECTS_DIR = $${DESTDIR}/.obj_libfm
MOC_DIR = $${DESTDIR}/.moc_libfm
RCC_DIR = $${DESTDIR}/.qrc_libfm

CONFIG(with_magick): include($${top_srcdir}/share/imagemagick.pri)
