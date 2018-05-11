QT+= core gui network dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lumina-files
TARGET_NAME = "Lumina Files"
VERSION = 5.9.1
TEMPLATE = app
DEPENDPATH += . src
INCLUDEPATH += . src
#OBJECTS_DIR = build
#MOC_DIR = build

RESOURCES += lumina-files.qrc
LIBS += -lmagic

# Input
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

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
isEmpty(DOCDIR) {
    DOCDIR = $$PREFIX/share/doc
}

target.path = $${PREFIX}/bin
desktop.files += $${TARGET}.desktop
desktop.path += $${PREFIX}/share/applications
icon.files += $${TARGET}.png
icon.path += $${PREFIX}/share/icons/hicolor/128x128

docs.path += $${DOCDIR}/$${TARGET}-$${VERSION}
docs.files += COPYING

DEFINES += APP=\"\\\"$${TARGET}\\\"\"
DEFINES += APP_NAME=\"\\\"$${TARGET_NAME}\\\"\"
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"


#trans.path += /usr/share/qtfm
#trans.files += translations/qtfm_da.qm \
#	       translations/qtfm_de.qm \
#	       translations/qtfm_es.qm \
#	       translations/qtfm_fr.qm \
#           translations/qtfm_it.qm \
#	       translations/qtfm_pl.qm \
#           translations/qtfm_ru.qm \
#           translations/qtfm_sr.qm \
#           translations/qtfm_sv.qm \
#           translations/qtfm_zh.qm \
#           translations/qtfm_zh_TW.qm

INSTALLS += target desktop icon docs
#trans

#OTHER_FILES += \
#    TODO.txt

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += Disks


OTHER_FILES += lumina-files.desktop




