TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

CONFIG(v7) {
    SUBDIRS += fm7
    fm7.depends += libfm
}

unix:!macx {
    SUBDIRS += settings
    settings.depends += libfm
    !CONFIG(no_dbus) {
        !CONFIG(no_tray) {
            SUBDIRS += tray
            tray.depends += libfm
        }
    }
    !CONFIG(no_launcher) {
        SUBDIRS += launcher
        launcher.depends += libfm
    }
}
