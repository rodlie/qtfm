QT+= core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(../qtfm.pri)

CONFIG(basic) {
    CONFIG += no_udisks no_tray no_dbus
}

macx: DEFINES += NO_DBUS NO_UDISKS

TARGET = $${QTFM_TARGET}
TARGET_NAME = $${QTFM_TARGET_NAME}
VERSION = $${QTFM_MAJOR}.$${QTFM_MINOR}.$${QTFM_PATCH}
TEMPLATE = app

DEPENDPATH += . src
INCLUDEPATH += . src ../libfm
LIBS += -L../libfm -lQtFM

HEADERS += src/mainwindow.h \
    src/mymodel.h \
    src/bookmarkmodel.h \
    src/icondlg.h \
    src/propertiesdlg.h \
    src/tabbar.h \
    src/mymodelitem.h \
    src/settingsdialog.h \
    src/customactionsmanager.h \
    src/processdialog.h \
    src/applicationdialog.h
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/mymodel.cpp \
    src/bookmarks.cpp \
    src/icondlg.cpp \
    src/propertiesdlg.cpp \
    src/tabbar.cpp \
    src/mymodelitem.cpp \
    src/settingsdialog.cpp \
    src/customactionsmanager.cpp \
    src/processdialog.cpp \
    src/applicationdialog.cpp \
    src/actiondefs.cpp \
    src/actiontriggers.cpp

OTHER_FILES += $${TARGET}.desktop
RESOURCES += ../$${TARGET}.qrc
macx {
    RESOURCES += bundle/adwaita.qrc
    ICON = images/QtFM.icns
    QMAKE_INFO_PLIST = Info.plist
}

DEFINES += APP=\"\\\"$${TARGET}\\\"\"
DEFINES += APP_NAME=\"\\\"$${TARGET_NAME}\\\"\"
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"

unix:!macx {
    target.path = $${PREFIX}/bin
    desktop.files += $${TARGET}.desktop
    desktop.path += $${PREFIX}/share/applications
    docs.path += $${DOCDIR}/$${TARGET}-$${VERSION}
    docs.files += ../LICENSE ../README.md ../AUTHORS ../ChangeLog
    man.files += qtfm.1
    man.path += $${MANDIR}/man1
    INSTALLS += target desktop docs man

    hicolor.files = hicolor
    hicolor.path = $${PREFIX}/share/icons
    INSTALLS += hicolor

    !CONFIG(no_udisks) {
        exists(../libdisks/libdisks.pro) {
            INCLUDEPATH += ../libdisks
            LIBS += -L../libdisks -lDisks
        } else {
            CONFIG += link_pkgconfig
            PKGCONFIG += Disks
        }
    }
    CONFIG(no_udisks): DEFINES += NO_UDISKS
    CONFIG(no_dbus) {
        DEFINES += NO_DBUS
        DEFINES += NO_UDISKS
    }
    !CONFIG(no_dbus) : QT += dbus
}

lessThan(QT_MAJOR_VERSION, 5): LIBS += -lmagic
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(deploy) : DEFINES += DEPLOY
