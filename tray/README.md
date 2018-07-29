# qtfm-tray

A system tray daemon used to manage storage/optical devices and actions releated to that.

## Features

* Show available storage/optical devices in system tray
* Auto mount storage/optical devices when added (not default)
* Auto play audio CD and video/audio DVD (not default)

## Roadmap

In the future this daemon can be used to cache/speed up varios things in QtFM. Support for more actions and devices is also possible.

## Usage

Start ``qtfm-tray`` during your X11 session, either manually or through XDG autostart. Please refer to the documentation for your DE/WM.

Settings can be modified in QtFM (Edit=>Settings=>System Tray). Auto play depends on mime types ``audio-cdda``, ``audio-dvd`` and ``video-dvd`` in the ``x-content`` mime category (Edit=>Settings=>Mime Types).
