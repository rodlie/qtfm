TEMPLATE = app
DEPENDPATH += . \
    src
INCLUDEPATH += . \
    src
OBJECTS_DIR = build
MOC_DIR = build

# Include widgets in QT 5
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

CONFIG += release warn_off thread
RESOURCES += resources.qrc
QT+= network
LIBS += -lmagic

TARGET = qtfm
target.path = /usr/bin
desktop.files += qtfm.desktop
desktop.path += /usr/share/applications
icon.files += images/qtfm.png
icon.path += /usr/share/pixmaps

docs.path += /usr/share/doc/qtfm
docs.files += README CHANGELOG COPYING

trans.path += /usr/share/qtfm
trans.files += translations/qtfm_da.qm \
	       translations/qtfm_de.qm \
	       translations/qtfm_es.qm \
	       translations/qtfm_fr.qm \
           translations/qtfm_it.qm \
	       translations/qtfm_pl.qm \
           translations/qtfm_ru.qm \
           translations/qtfm_sr.qm \
           translations/qtfm_sv.qm \
           translations/qtfm_zh.qm \
           translations/qtfm_zh_TW.qm

INSTALLS += target desktop icon docs trans

#OTHER_FILES += \
#    TODO.txt


# tmp for libdisks
INCLUDEPATH += /usr/local/include/libdisks
LIBS += -L/usr/local/lib -lDisks
QT += dbus








