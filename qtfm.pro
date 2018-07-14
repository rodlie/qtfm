TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

CONFIG(basic) {
    CONFIG += no_udisks no_tray
}
!CONFIG(no_udisks) {
    SUBDIRS += libdisks
    fm.depends += libdisks
}
!CONFIG(no_tray) {
    SUBDIRS += tray
    tray.depends += libdisks libfm
}
OTHER_FILES += .travis.yml qtfm.SlackBuild
