TEMPLATE = subdirs
CONFIG += ordered
exists(libdisks) {
    SUBDIRS += libdisks
}
SUBDIRS += fm
contains(CONFIG, with_power) {
    message("Enabled qtfm-power")
    exists(libpower): SUBDIRS += libpower
    SUBDIRS += power
}
OTHER_FILES += README.md
