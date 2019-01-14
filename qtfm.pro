TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

unix:!macx {
    CONFIG(basic) {
        CONFIG += no_udisks no_tray no_dbus
    }
    !CONFIG(no_udisks) {
        SUBDIRS += libdisks
        fm.depends += libdisks
    }
    !CONFIG(no_tray) {
        SUBDIRS += tray libdisks
        tray.depends += libdisks libfm
    }
}
lessThan(QT_MAJOR_VERSION, 5): warning("Qt4 is deprecated, please consider upgrading to Qt5 if possible.")
