TEMPLATE = subdirs
CONFIG += ordered
exists(lib) {
    SUBDIRS += lib
}
SUBDIRS += fm
