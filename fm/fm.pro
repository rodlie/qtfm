QT+= widgets
include(../qtfm.pri)

TARGET = $${QTFM_TARGET}
TARGET_NAME = $${QTFM_TARGET_NAME}
VERSION = $${QTFM_MAJOR}.$${QTFM_MINOR}.$${QTFM_PATCH}
TEMPLATE = app

DEPENDPATH += . src
INCLUDEPATH += . src $${top_srcdir}/libfm

HEADERS += \
    src/mainwindow.h \
    src/bookmarkmodel.h \
    src/tabbar.h \
    src/settingsdialog.h

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/bookmarks.cpp \
    src/tabbar.cpp \
    src/settingsdialog.cpp \
    src/actiondefs.cpp \
    src/actiontriggers.cpp

OTHER_FILES += $${TARGET}.desktop
RESOURCES += ../$${TARGET}.qrc

macx {
    LIBS += -L$${top_builddir}/libfm -lQtFM
    DEFINES += NO_DBUS NO_UDISKS
    RESOURCES += bundle/adwaita.qrc
    ICON = images/QtFM.icns
    QMAKE_INFO_PLIST = Info.plist
}

DEFINES += APP=\"\\\"$${TARGET}\\\"\"
DEFINES += APP_NAME=\"\\\"$${TARGET_NAME}\\\"\"
DEFINES += APP_VERSION=\"\\\"$${VERSION}\\\"\"

unix:!macx {
    DESTDIR = $${top_builddir}/bin
    OBJECTS_DIR = $${DESTDIR}/.obj_fm
    MOC_DIR = $${DESTDIR}/.moc_fm
    RCC_DIR = $${DESTDIR}/.qrc_fm
    LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM
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

    CONFIG(no_dbus) {
        DEFINES += NO_DBUS
        DEFINES += NO_UDISKS
    }
    !CONFIG(no_dbus) : QT += dbus
    !CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
}

CONFIG(with_magick): include($${top_srcdir}/share/imagemagick.pri)
