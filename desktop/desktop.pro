lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")

TEMPLATE = subdirs
CONFIG -= ordered
SUBDIRS += launcher
