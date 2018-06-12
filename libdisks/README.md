# libdisks

Udisks library for Qt.

```
mkdir build && cd build
qmake PREFIX=/usr LIBSUFFIX=64 .. && make
make INSTALL_ROOT=/tmp/pkg install
```
```
pkg
└── usr
    ├── include
    │   └── libdisks
    │       └── disks.h
    ├── lib64
    │   ├── libDisks.so -> libDisks.so.1.0.0
    │   ├── libDisks.so.1 -> libDisks.so.1.0.0
    │   ├── libDisks.so.1.0 -> libDisks.so.1.0.0
    │   ├── libDisks.so.1.0.0
    │   └── pkgconfig
    │       └── Disks.pc
    └── share
        └── doc
            └── libdisks-1.0.0
                ├── LICENSE
                └── README.md
```
