QT+= core gui network dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtfm
TARGET_NAME = "QtFM"
VERSION = 6.0.0
TEMPLATE = app

DEPENDPATH += . src
INCLUDEPATH += . src

#OBJECTS_DIR = .obj
#MOC_DIR = .moc

HEADERS += src/mainwindow.h \
    src/mymodel.h \
    src/bookmarkmodel.h \
    src/progressdlg.h \
    src/icondlg.h \
    src/propertiesdlg.h \
    src/fileutils.h \
    src/tabbar.h \
    src/mymodelitem.h \
    src/settingsdialog.h \
    src/customactionsmanager.h \
    src/processdialog.h \
    src/desktopfile.h \
    src/applicationdialog.h \
    src/properties.h \
    src/mimeutils.h \
    src/aboutdialog.h
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/mymodel.cpp \
    src/bookmarks.cpp \
    src/progressdlg.cpp \
    src/fileutils.cpp \
    src/icondlg.cpp \
    src/propertiesdlg.cpp \
    src/tabbar.cpp \
    src/mymodelitem.cpp \
    src/settingsdialog.cpp \
    src/customactionsmanager.cpp \
    src/processdialog.cpp \
    src/desktopfile.cpp \
    src/applicationdialog.cpp \
    src/properties.cpp \
    src/mimeutils.cpp \
    src/aboutdialog.cpp \
    src/actiondefs.cpp \
    src/actiontriggers.cpp

OTHER_FILES += $${TARGET}.desktop
RESOURCES += $${TARGET}.qrc

DEFINES += APP=\"\\\"$${TARGET}\\\"\"
DEFINES += APP_NAME=\"\\\"$${TARGET_NAME}\\\"\"
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"

exists(../qtfm.pri) {
    include(../qtfm.pri)
}

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
isEmpty(DOCDIR) {
    DOCDIR = $$PREFIX/share/doc
}

target.path = $${PREFIX}/bin
desktop.files += $${TARGET}.desktop
desktop.path += $${PREFIX}/share/applications
docs.path += $${DOCDIR}/$${TARGET}-$${VERSION}
docs.files += ../COPYING
INSTALLS += target desktop docs

exists(../lib) {
    message("Using embedded libdisks")
    INCLUDEPATH += ../lib
    LIBS += -L../lib -lDisks
} else {
    message("Using external libdisks")
    CONFIG += link_pkgconfig
    PKGCONFIG += Disks
}
LIBS += -lmagic

