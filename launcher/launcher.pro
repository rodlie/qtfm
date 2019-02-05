include($${top_srcdir}/share/qtfm.pri)

QT += concurrent

TARGET = qtfm-launcher
TEMPLATE = app

SOURCES += main.cpp dialog.cpp
HEADERS += dialog.h
RESOURCES += $${top_srcdir}/share/$${QTFM_TARGET}.qrc
INCLUDEPATH += $${top_srcdir}/libfm

unix:!macx {
    DESTDIR = $${top_builddir}/bin
    OBJECTS_DIR = $${DESTDIR}/.obj_launcher
    MOC_DIR = $${DESTDIR}/.moc_launcher
    RCC_DIR = $${DESTDIR}/.qrc_launcher
    LIBS += -L$${top_builddir}/lib$${LIBSUFFIX} -lQtFM

    target.path = $${PREFIX}/bin
    #desktop.files += $${TARGET}.desktop
    #desktop.path += $${PREFIX}/share/applications
    #man.files += qtfm-launcher.1
    #man.path += $${MANDIR}/man1
    INSTALLS += target
    #desktop man
    !CONFIG(staticlib): QMAKE_RPATHDIR += $ORIGIN/../lib$${LIBSUFFIX}
}
