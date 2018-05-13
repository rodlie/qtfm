TEMPLATE = subdirs
CONFIG += ordered
exists(lib) {
    SUBDIRS += lib
}
SUBDIRS += fm
OTHER_FILES += README.md
