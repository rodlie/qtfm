![screenshot](qtfm.png)

QtFM, originally created by [Wittfella](http://www.qtfm.org/) in 2010, and further developed by [Michal Rost](http://qt-apps.org/content/show.php/QtFM?content=158787) in 2012 has found a new home. This project [continues](https://github.com/rodlie/qtfm/compare/cea6fbacc6ebb52032077bea03b2891a67f85c3f...master) from the v5.9 source code released in 2013.

## Links

 * [Releases](https://github.com/rodlie/qtfm/releases)
 * [Milestones](https://github.com/rodlie/qtfm/milestones)
 * [Issues](https://github.com/rodlie/qtfm/issues)
 
## Build

Download the latest [release](https://github.com/rodlie/qtfm/releases), then make sure you have the required dependencies.

* Requires [Qt](http://qt.io) 4.8+ (5.6+ recommended) (core gui network dbus widgets) development files
* Requires [file](http://darwinsys.com/file/) development files
* Requires [hicolor-icon-theme](https://www.freedesktop.org/wiki/Software/icon-theme/)
* Optional [udisks](https://www.freedesktop.org/wiki/Software/udisks/) at run time
* Optional [tango-icon-theme](http://tango.freedesktop.org) (or similar)

Extract and create a temporary build directory:
```
tar xvf qtfm-VERSION.tar.xz
cd qtfm-VERSION
mkdir build && cd build
```

Now you just need to build it:
```
qmake CONFIG+=release .. && make
```
You can now run the binary ``fm/qtfm`` or install to /usr/local:
```
sudo make install
```

## Packaging

Same as above, but you should use ``PREFIX=`` and ``INSTALL_ROOT=``.

Example:

```
cd qtfm-VERSION
mkdir build && cd build
qmake CONFIG+=release PREFIX=/usr ..
make -jX
make INSTALL_ROOT=/package_temp_path install
```
