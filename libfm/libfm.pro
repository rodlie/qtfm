QT += core dbus gui widgets

TARGET = QtFM
VERSION = 1.2.0

TEMPLATE = lib
SOURCES += \
    desktopfile.cpp \
    fileutils.cpp \
    mimeutils.cpp \
    properties.cpp \
    progressdlg.cpp \
    mymodelitem.cpp \
    mymodel.cpp \
    fm.cpp
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
    fm.h
!CONFIG(no_dbus) {
    HEADERS += service.h
    QT += dbus
}

exists(../qtfm.pri) {
    include(../qtfm.pri)
}
