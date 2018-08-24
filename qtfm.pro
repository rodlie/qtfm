lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")

TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

CONFIG(basic) {
    CONFIG += no_udisks no_tray
}
!CONFIG(no_tray) {
    SUBDIRS += tray
    tray.depends += libfm
}

!CONFIG(no_desktop) {
    SUBDIRS += desktop
    desktop.depends += libfm
}

CONFIG(v7) {
    SUBDIRS += qtfm7
    qtfm7.depends += libfm
}
