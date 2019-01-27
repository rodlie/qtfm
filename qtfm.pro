TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm libdisks fm
fm.depends += libfm

unix:!macx {
    !CONFIG(no_udisks): fm.depends += libdisks
    !CONFIG(no_tray) {
        SUBDIRS += tray
        tray.depends += libdisks libfm
    }
}
CONFIG(deploy): SUBDIRS -= libdisks

lessThan(QT_MAJOR_VERSION, 5): warning("Qt4 is deprecated, please consider upgrading to Qt5 if possible.")
