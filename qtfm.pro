TEMPLATE = subdirs
CONFIG += ordered
exists(libdisks) {
    SUBDIRS += libdisks
}
SUBDIRS += fm
OTHER_FILES += README.md
