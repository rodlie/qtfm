<p align="left"><img src="fm/hicolor/scalable/apps/qtfm.svg" alt="QList" height="100px"></p>

[![GitHub release](https://img.shields.io/github/release/rodlie/qtfm.svg)](https://github.com/rodlie/qtfm/releases)
[![Github commits (since latest release)](https://img.shields.io/github/commits-since/rodlie/qtfm/latest.svg)](https://github.com/rodlie/qtfm)
[![Build Status](https://travis-ci.org/rodlie/qtfm.svg?branch=master)](https://travis-ci.org/rodlie/qtfm)
[![Github All Releases](https://img.shields.io/github/downloads/rodlie/qtfm/total.svg)](https://github.com/rodlie/qtfm/releases)

![screenshot1](fm/images/screenshot.png)

Lightweight file manager using Qt.

 * Desktop (theme/applications/mime) integration
 * Customizable interface
 * Powerful custom command system
 * Customizable key bindings
 * Drag & drop functionality
 * Tabs
 * Udisks support
 * System tray daemon
   * Mount/Unmount devices
   * Auto play CD/DVD

## Links

 * [Releases](https://github.com/rodlie/qtfm/releases)
 * [Milestones](https://github.com/rodlie/qtfm/milestones)
 * [Issues](https://github.com/rodlie/qtfm/issues)

## Default shortcuts

| Action                      | Shortcut    |
|:---------------------------:|:-----------:|
| New Window                  | [ctrl+n]    |
| Open Tab                    | [ctrl+t]    |
| Close Tab                   | [ctrl+w]    |
| Copy Action                 | [ctrl+c]    |
| Paste Action                | [ctrl+v]    |
| Up Action                   | [alt+up]    |
| Back Action                 | [backspace] |
| Toggle Show Hidden Files    | [ctrl+h]    |
| Move to Trash               | [del]       |
| Delete                      | [shift+del] |
| Open Terminal               | [F1]        |
| Quit/Close                  | [ctrl+q]    |
| Rename Action               | [F2]        |
| Zoom Out                    | [ctrl+-]    |
| Zoom In                     | [ctrl++]    |
| Focus Path                  | [ctrl+l]    |
| Toggle Icon View            | [F3]        |
| Toggle Detail View          | [F4]        |
| Drag and Drop Move Modifier | [shift]     |
| Drag and Drop Copy Modifier | [ctrl]      |
| Drag and Drop Ask Action    | [alt]       |

All shortcuts can be modified.

## Packages

QtFM is available in several distributions:

[![Packaging status](https://repology.org/badge/vertical-allrepos/qtfm.svg)](https://repology.org/metapackage/qtfm)

## Build

Download the latest [release](https://github.com/rodlie/qtfm/releases), then make sure you have the required dependencies.

* Requires [Qt](http://qt.io) 4.8+ (5.6+ recommended)
* Requires libmagic from [file](http://darwinsys.com/file/) **if using Qt 4**.
* Requires [hicolor-icon-theme](https://www.freedesktop.org/wiki/Software/icon-theme/)
* Requires [adwaita-icon-theme](https://github.com/GNOME/adwaita-icon-theme) or similar
* Requires [libinotify](https://github.com/libinotify-kqueue/libinotify-kqueue) **if using FreeBSD**
* Optional [udisks](https://www.freedesktop.org/wiki/Software/udisks/)/[bsdisks](https://www.freshports.org/sysutils/bsdisks/) at run-time

Download and extract:
```
tar xvf qtfm-VERSION.tar.gz
cd qtfm-VERSION
```

Or use git:
```
git clone https://github.com/rodlie/qtfm
cd qtfm
```

Now you just need to build it:

Optional build options:

 * ``CONFIG+=no_udisks`` : This will disable udisks support in QtFM
 * ``CONFIG+=no_tray`` : This will disable qtfm-tray
 * ``CONFIG+=no_dbus`` : This will disable D-Bus session features in QtFM
 * ``CONFIG+=release`` : Disable debug output (recommended)
 * ``PREFIX=`` : Install prefix, where things are installed when running ``make install``
 * ``DOCDIR=`` : Location for documentation (default is PREFIX/share/doc)
 * ``MANDIR=`` : Location for man pages (default is PREFIX/share/man)
 * ``XDGDIR=`` : Location of XDG (default is /etc/xdg or PREFIX/etc/xdg)

Doing a normal build:
```
mkdir build && cd build
qmake CONFIG+=release ..
make
```

You can run the binary ``fm/qtfm`` or install to ``/usr/local``:
```
sudo make install
```

## Packaging

Same as above, but you should use ``PREFIX=`` and ``INSTALL_ROOT=``.

Example:

```
mkdir build && cd build
qmake CONFIG+=release PREFIX=/usr ..
make -jX
make INSTALL_ROOT=/package_temp_path install
```

## License

 * QtFM is licensed under the GPL-2.0 License
 * libdisks is licensed under a BSD 3-Clause License
 * libfm is a mix of GPL-2.0 and BSD 3-Clause License (goal is BSD 3-Clause License)
 * qtfm-tray is licensed under a BSD 3-Clause License (but depends on libfm that includes GPL-2.0 code)

 All new source code will be BSD 3-Clause License, modifications to existing source code in fm/ will be GPL-2.0.

## Related

 * **[powerdwarf](https://github.com/rodlie/powerdwarf)** : Desktop Independent Power Manager.
 * **[Openbox](http://openbox.org/wiki/Main_Page)** : Openbox is a highly configurable, next generation window manager with extensive standards support.
 * **[tint2](https://gitlab.com/o9000/tint2)** : A lightweight panel/taskbar for Linux and BSD.
 * **[qterminal](https://github.com/lxqt/qterminal)** : A lightweight Qt-based terminal emulator.
