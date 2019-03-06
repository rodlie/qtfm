CONFIG(with_ffmpeg): PKGCONFIG += libavdevice \
                                  libswscale \
                                  libavformat \
                                  libavcodec \
                                  libavutil
CONFIG(with_ffmpeg): DEFINES += WITH_FFMPEG

