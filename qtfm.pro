TEMPLATE = subdirs
CONFIG += ordered
exists(libdisks) {
    SUBDIRS += libdisks
}
exists(libpower) {
    SUBDIRS += libpower
}
SUBDIRS += fm power
OTHER_FILES += README.md
