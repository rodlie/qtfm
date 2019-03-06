include($${top_srcdir}/share/qtfm.pri)

QT += dbus widgets
CONFIG(staticlib): QT += concurrent

TARGET = qtfm-tray
TEMPLATE = app

SOURCES += main.cpp systray.cpp
HEADERS += systray.h
RESOURCES += tray.qrc

INCLUDEPATH += $${top_srcdir}/libfm
LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM

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

!CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
CONFIG(staticlib): CONFIG(with_magick): include($${top_srcdir}/share/imagemagick.pri)
CONFIG(staticlib): CONFIG(with_ffmpeg): include($${top_srcdir}/share/ffmpeg.pri)
