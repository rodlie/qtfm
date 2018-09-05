lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")

TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

!CONFIG(no_utils) {
    SUBDIRS += utils
    utils.depends += libfm
}

CONFIG(v7) {
    SUBDIRS += qtfm7
    qtfm7.depends += libfm
}
