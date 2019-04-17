include($${top_srcdir}/share/qtfm.pri)

TARGET = qtfm-settings-mime
TEMPLATE = app

SOURCES += main.cpp settingsdialog.cpp
HEADERS += settingsdialog.h
INCLUDEPATH += $${top_srcdir}/libfm

unix:!macx {
    DESTDIR = $${top_builddir}/bin
    OBJECTS_DIR = $${DESTDIR}/.obj_settings-mime
    MOC_DIR = $${DESTDIR}/.moc_settings-mime
    RCC_DIR = $${DESTDIR}/.qrc_settings-mime
    LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM
    !CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}

    target.path = $${PREFIX}/bin
    desktop.files += $${TARGET}.desktop
    desktop.path += $${PREFIX}/share/applications
    INSTALLS += target desktop
}

CONFIG(staticlib): CONFIG(with_magick): include($${top_srcdir}/share/imagemagick.pri)
CONFIG(staticlib): CONFIG(with_ffmpeg): include($${top_srcdir}/share/ffmpeg.pri)
