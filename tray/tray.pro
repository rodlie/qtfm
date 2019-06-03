include($${top_srcdir}/share/qtfm.pri)

QT += dbus widgets
TARGET = qtfm-tray
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += tray.qrc

INCLUDEPATH += $${top_srcdir}/libfm

DESTDIR = $${top_builddir}/bin
OBJECTS_DIR = $${DESTDIR}/.obj_tray
MOC_DIR = $${DESTDIR}/.moc_tray
RCC_DIR = $${DESTDIR}/.qrc_tray

target.path = $${PREFIX}/bin
desktop.path = $${XDGDIR}/autostart
desktop.files = $${TARGET}.desktop
man.files += qtfm-tray.1
man.path += $${MANDIR}/man1
INSTALLS += target desktop man

# Don't link against libfm if static (too many 3rdparty depends)
CONFIG(staticlib) {
    SOURCES += \
        $${top_srcdir}/libfm/common.cpp \
        $${top_srcdir}/libfm/disks.cpp \
        $${top_srcdir}/libfm/udisks2.cpp \
        $${top_srcdir}/libfm/fileutils.cpp \
        $${top_srcdir}/libfm/desktopfile.cpp \
        $${top_srcdir}/libfm/properties.cpp \
        $${top_srcdir}/libfm/mimeutils.cpp
    HEADERS += \
        $${top_srcdir}/libfm/disks.h \
        $${top_srcdir}/libfm/disks.h \
        $${top_srcdir}/libfm/udisks2.h \
        $${top_srcdir}/libfm/fileutils.h \
        $${top_srcdir}/libfm/desktopfile.h \
        $${top_srcdir}/libfm/properties.h \
        $${top_srcdir}/libfm/mimeutils.h
}
# Link against libfm if shared
!CONFIG(staticlib) {
    LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM
    QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
}
