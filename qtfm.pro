TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm settings
fm.depends += libfm
settings.depends += libfm

CONFIG(v7) {
    SUBDIRS += fm7
    fm7.depends += libfm
}

unix:!macx {
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
