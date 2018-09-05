lessThan(QT_MAJOR_VERSION, 5): error("Qt4 is not supported anymore.")

TEMPLATE = subdirs
CONFIG -= ordered

!CONFIG(no_tray) : SUBDIRS += tray
!CONFIG(no_launcher): SUBDIRS += launcher
