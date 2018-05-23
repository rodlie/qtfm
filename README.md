[![Build Status](https://travis-ci.org/rodlie/qtfm.svg?branch=master)](https://travis-ci.org/rodlie/qtfm)

![screenshot](qtfm.png)

Lightweight file manager using Qt.

 * Desktop (theme/applications/mime) integration
 * Customizable interface
 * Powerful custom command system
 * Customizable key bindings
 * Drag & drop functionality
 * Tabs
 * Udisks device support

## QtFM Power

![screenshot2](qtfm-power.png)

Experimental lightweight power daemon/systray with support for UPower and freedesktop.

 * org.freedesktop.ScreenSaver support
 * org.freedesktop.PowerManagement support
 * Critical battery action
 * auto sleep action
 * lid action
   
This is a seperate application called ``qtfm-power``, and should be started during the X11 session in Fluxbox ``~/.fluxbox/startup`` or Openbox ``~/.config/openbox/autostart``.

## Links

 * [Releases](https://github.com/rodlie/qtfm/releases)
 * [Milestones](https://github.com/rodlie/qtfm/milestones)
 * [Issues](https://github.com/rodlie/qtfm/issues)
 * [Slackware](https://slackbuilds.org/repository/14.2/system/qtfm/)
 * [Arch Linux](https://aur.archlinux.org/packages/qtfm/)
 * [Gentoo](https://packages.gentoo.org/packages/x11-misc/qtfm)
 
## Build

Download the latest [release](https://github.com/rodlie/qtfm/releases), then make sure you have the required dependencies.

* Requires [Qt](http://qt.io) 4/5 development files
* Requires [file](http://darwinsys.com/file/) development files **if using Qt 4**
* Requires xss (X11 Screen Saver extension client library) if building qtfm-power
* Requires [hicolor-icon-theme](https://www.freedesktop.org/wiki/Software/icon-theme/) at run time
* Optional [udisks](https://www.freedesktop.org/wiki/Software/udisks/) at run time
* Optional [upower](https://upower.freedesktop.org/) at run time if using qtfm-power
* Optional [tango-icon-theme](http://tango.freedesktop.org) (or similar) at run time
* Optional [xscreensaver](https://www.jwz.org/xscreensaver/) at run time if using qtfm-power

Download and extract:
```
tar xvf qtfm-VERSION.tar.xz
cd qtfm-VERSION
```

Or use git:
```
git clone https://github.com/rodlie/qtfm
cd qtfm
git submodule update --init --recursive
```

Now you just need to build it:

```
mkdir build && cd build
```

```
qmake CONFIG+=release ..
```
_Add **CONFIG+=with_power** to enable qtfm-power_

```
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
qmake CONFIG+=release CONFIG+=with_power PREFIX=/usr ..
make -jX
make INSTALL_ROOT=/package_temp_path install
```
```
package
└── usr
    ├── bin
    │   ├── qtfm
    │   ├── qtfm-power
    │   └── qtfm-power-settings
    └── share
        ├── applications
        │   ├── qtfm-power-settings.desktop
        │   └── qtfm.desktop
        └── doc
            └── qtfm-6.0.0
                ├── LICENSE
                └── README.md
```
* You can change the doc location with ``DOCDIR=``, default is ``PREFIX/share/doc``.
