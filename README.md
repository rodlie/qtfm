<p align="left"><img src="share/hicolor/scalable/apps/qtfm.svg" alt="QList" height="100px"></p>

[![GitHub release](https://img.shields.io/github/release/rodlie/qtfm.svg)](https://github.com/rodlie/qtfm/releases)
[![Github commits (since latest release)](https://img.shields.io/github/commits-since/rodlie/qtfm/latest.svg)](https://github.com/rodlie/qtfm)
[![Build Status](https://travis-ci.org/rodlie/qtfm.svg?branch=master)](https://travis-ci.org/rodlie/qtfm)

![screenshot1](docs/screenshot.png)

Lightweight file manager using Qt.

  * Desktop (theme/applications/mime) integration
  * Customizable interface
  * Powerful custom command system
  * Customizable key bindings
  * Drag & drop functionality
  * Tabs
  * Udisks support
  * System tray daemon (qtfm-tray, see man for more info)
    * Show available storage/optical devices in system tray
    * Auto mount storage/optical devices when added (not default)
    * Auto play audio CD and video/audio DVD (not default)
  * Application launcher (qtfm-launcher) for Openbox and similar
  * Extensive thumbnail support
    * Supports all common (and uncommon) image formats through ImageMagick
    * Supports PDF and related documents
    * Supports all common (and uncommon) video formats through FFmpeg
    * Supports embedded coverart in audio

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
| Refresh View                | [F5]        |
| Drag and Drop Move Modifier | [shift]     |
| Drag and Drop Copy Modifier | [ctrl]      |
| Drag and Drop Ask Action    | [alt]       |

All shortcuts can be modified.

## Packages

QtFM is available in several distributions:

[![Packaging status](https://repology.org/badge/vertical-allrepos/qtfm.svg)](https://repology.org/metapackage/qtfm)

## Build

Download the latest [release](https://github.com/rodlie/qtfm/releases), then make sure you have the required dependencies.

* Requires [QtBase](http://qt.io) 5.2+ (5.6.3/5.9.7/5.12.x recommended)
* Requires [hicolor-icon-theme](https://www.freedesktop.org/wiki/Software/icon-theme/)
* Requires [adwaita-icon-theme](https://github.com/GNOME/adwaita-icon-theme) or similar
* Requires [libinotify](https://github.com/libinotify-kqueue/libinotify-kqueue) **if using FreeBSD/NetBSD/macOS**
* Optional [udisks](https://www.freedesktop.org/wiki/Software/udisks/)/[bsdisks](https://www.freshports.org/sysutils/bsdisks/) at run-time
* Optional [ImageMagick (Magick++)](http://imagemagick.org) 6.9/7.x for improved thumbnail support
* Optional [FFmpeg](https://ffmpeg.org) 3+ for video/audio thumbnails
  * libavdevice
  * libswscale
  * libavformat
  * libavcodec
  * libavutil

Download and extract:
```
tar xvf qtfm-6.x.x.tar.gz
cd qtfm-6.x.x
```

Or use git:
```
git clone https://github.com/rodlie/qtfm
cd qtfm
```
Optional build options:

 * ``CONFIG+=with_magick`` : This will enable ImageMagick support
   * ``MAGICK_PC=`` : Custom Magick++ pkg-config .pc file
   * ``CONFIG+=magick7`` : Use ImageMagick 7
 * ``CONFIG+=with_ffmpeg`` : This will enable FFmpeg support (NOTE! requires ImageMagick)
 * ``CONFIG+=no_tray`` : This will disable qtfm-tray
 * ``CONFIG+=no_dbus`` : This will disable D-Bus session and udisks features in (lib)QtFM
 * ``CONFIG+=no_launcher`` : Disable qtfm-launcher (freedesktop application launcher)
 * ``CONFIG+=with_includes`` : Install library includes and pkg-config
 * ``CONFIG+=staticlib`` : Force static library
 * ``PREFIX=`` : Install prefix, where things are installed when running ``make install``
 * ``DOCDIR=`` : Location for documentation (default is PREFIX/share/doc)
 * ``MANDIR=`` : Location for man pages (default is PREFIX/share/man)
 * ``XDGDIR=`` : Location of XDG (default is /etc/xdg or PREFIX/etc/xdg)
 * ``LIBDIR=`` : Library location (default is ``$PREFIX/lib$LIBSUFFIX``)


Doing a normal build:
```
mkdir build && cd build
qmake ..
make
```

You can run the binary ``bin/qtfm`` or install to ``/usr/local``:
```
sudo make install
```

## Packaging

Same as above, but you should use ``PREFIX=`` and ``INSTALL_ROOT=``.

Example:

```
mkdir build && cd build
qmake PREFIX=/usr LIBSUFFIX=64 CONFIG+=with_magick CONFIG+=with_ffmpeg CONFIG+=with_includes ..
make -jX
make INSTALL_ROOT=/package_temp_path install
```

## License

QtFM is licensed under the GPL-2.0 License.

## Related

 * **[powerkit](https://github.com/rodlie/powerkit)** : Desktop Independent Power Manager.
 * **[Openbox](http://openbox.org/wiki/Main_Page)** : Openbox is a highly configurable, next generation window manager with extensive standards support.
