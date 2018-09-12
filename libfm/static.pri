unix:!linux { LIBS += -linotify }
isEmpty(MAGICK_PKGCONFIG) {
    PKGCONFIG += Magick++
} else {
    PKGCONFIG += $${MAGICK_PKGCONFIG}
}
!CONFIG(no_ffmpeg): PKGCONFIG += libavdevice \
                                 libswscale \
                                 libavformat \
                                 libavcodec \
                                 libavutil
