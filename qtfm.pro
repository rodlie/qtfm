TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += libfm fm
fm.depends += libfm

unix:!macx {
    !CONFIG(no_tray) {
        SUBDIRS += tray
        tray.depends += libfm
    }
}

lessThan(QT_MAJOR_VERSION, 5): warning("Qt4 is deprecated, please consider upgrading to Qt5 if possible.")
