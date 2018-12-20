#!/bin/sh
install_name_tool -add_rpath "${HOME}/.local/share/qt/5.12.0/clang_64/lib" fm/qtfm.app/Contents/MacOS/qtfm
