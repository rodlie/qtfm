QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    common.cpp

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
    propertiesdlg.h

unix:!macx {
    !CONFIG(no_dbus) {
        SOURCES += disks.cpp udisks2.cpp
        HEADERS += disks.h udisks2.h service.h
        QT += dbus
    }
}

exists(../qtfm.pri) {
    include(../qtfm.pri)
}

lessThan(QT_MAJOR_VERSION, 5): LIBS += -lmagic
