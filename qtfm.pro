TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

CONFIG(basic) {
    CONFIG += no_udisks no_tray no_open
}

!CONFIG(no_udisks) {
    SUBDIRS += libdisks
    fm.depends += libdisks
}
!CONFIG(no_tray) {
    SUBDIRS += tray
    tray.depends += libdisks libfm
}
!CONFIG(no_open) {
    SUBDIRS += open
    open.depends += libfm
}
OTHER_FILES += .travis.yml
