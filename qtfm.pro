TEMPLATE = subdirs
CONFIG += ordered
exists(libdisks/libdisks.pro): SUBDIRS += libdisks
exists(libpower/libpower.pro): SUBDIRS += libpower
SUBDIRS += fm
OTHER_FILES += README.md LICENSE
