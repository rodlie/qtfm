TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

unix:!macx {
    !CONFIG(no_dbus) {
        !CONFIG(no_tray) {
            SUBDIRS += tray
            tray.depends += libfm
        }
    }
}
