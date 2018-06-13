TEMPLATE = subdirs
CONFIG += ordered
exists(libdisks/libdisks.pro): SUBDIRS += libdisks
SUBDIRS += fm tray open
OTHER_FILES += README.md LICENSE
