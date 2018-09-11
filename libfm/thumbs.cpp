/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "thumbs.h"
#include <Magick++.h>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "common.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}

Thumbs::Thumbs(QObject *parent) : QObject(parent)
{
    Magick::InitializeMagick(Q_NULLPTR);
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();

    moveToThread(&t);
    t.start();
}

Thumbs::~Thumbs()
{
    t.quit();
    t.wait();
}

void Thumbs::generateIcon(QString file, QString mimetype)
{
    if (!QFile::exists(file)) { return; }
    QMetaObject::invokeMethod(this,"procIcon",
                              Q_ARG(QString, file),
                              Q_ARG(QString, mimetype));
}


void Thumbs::procIcon(QString file, QString mimetype)
{
    if (!QFile::exists(file)) { return; }
    qDebug() << "procIcon" << file << mimetype;

    QByteArray result;
    try {
        QString bgColor = "black";
        if (mimetype == "application/pdf") { bgColor = "white"; }
        Magick::Image background(Magick::Geometry(128, 128), Magick::Color(bgColor.toStdString()));
#ifdef MAGICK7
        background.alpha(true);
#else
        background.matte(true);
#endif
        if (mimetype != "application/pdf") {
            background.backgroundColor(background.pixelColor(0,0));
            background.transparent(background.pixelColor(0,0));
        }

        Magick::Image thumb;
        QString filename = file;
        QStringList videos;
        videos << "mpeg" << "vob" << "mov" << "avi" << "mkv" << "mp4" << "divx" << "flv";

        QFileInfo fileinfo(filename);
        if (videos.contains(fileinfo.suffix(), Qt::CaseInsensitive)) {
            //thumb.read(QString("%1[100]").arg(filename).toUtf8().data());
            procFF(filename);
            return;

        } else {
            thumb.read(filename.toUtf8().data());
        }
        thumb.scale(Magick::Geometry(128, 128));
        if (thumb.depth()>8) { thumb.depth(8); }
        int offsetX = 0;
        int offsetY = 0;
        if (thumb.columns()<background.columns()) {
            offsetX = (int)(background.columns()-thumb.columns())/2;
        }
        if (thumb.rows()<background.rows()) {
            offsetY = (int)(background.rows()-thumb.rows())/2;
        }
        background.composite(thumb, offsetX, offsetY, MagickCore::OverCompositeOp);
        background.magick("BMP");
        Magick::Blob buffer;
        background.write(&buffer);
        result = QByteArray((char*)buffer.data(), buffer.length());
    }
    catch(Magick::Error &error_ ) {
        qWarning() << error_.what();
    }
    catch(Magick::Warning &warn_ ) {
        qWarning() << warn_.what();
    }
    if (result.length()>0) {
        emit generatedIcon(file, result);
    }
}

void Thumbs::procFF(QString file)
{
    if (file.isEmpty()) { return; }

    AVCodecContext  *pCodecCtx;
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVCodec * pCodec;
    AVFrame *pFrame, *pFrameRGB;

    if (avformat_open_input(&pFormatCtx,file.toUtf8().data(),
                           Q_NULLPTR,Q_NULLPTR) != 0) { return; }
    if (avformat_find_stream_info(pFormatCtx,
                                  Q_NULLPTR) < 0) { return; }

    av_dump_format(pFormatCtx, 0, file.toUtf8().data(), 0);
    int videoStream = 1;

    for (int i=0; i < (int)pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) { return; }
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec =avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == Q_NULLPTR) { return; }
    if (avcodec_open2(pCodecCtx,
                     pCodec,
                     Q_NULLPTR) < 0) { return; }

    pFrame    = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    uint8_t *buffer;
    int numBytes;

    AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
    numBytes = avpicture_get_size(pFormat,
                                  pCodecCtx->width,
                                  pCodecCtx->height);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
    avpicture_fill((AVPicture*)pFrameRGB,buffer,
                   pFormat,
                   pCodecCtx->width,
                   pCodecCtx->height);

    int res;
    int frameFinished;
    AVPacket packet;
    int currentFrame = 0;
    bool fetchFrame = false;
    double fps = av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
    double dur = static_cast<double>(pFormatCtx->duration)/AV_TIME_BASE;
    int maxFrame = qRound((dur*fps)/2);

    while((res = av_read_frame(pFormatCtx,&packet)>=0)) {
        if (currentFrame>=maxFrame) { fetchFrame = true; }
        if (packet.stream_index == videoStream){
            if (!fetchFrame) {
                currentFrame++;
                continue;
            }
            avcodec_decode_video2(pCodecCtx,
                                  pFrame,
                                  &frameFinished,
                                  &packet);
            if (frameFinished) {
                struct SwsContext * img_convert_ctx;
                img_convert_ctx = sws_getCachedContext(Q_NULLPTR,
                                                       pCodecCtx->width,
                                                       pCodecCtx->height,
                                                       pCodecCtx->pix_fmt,
                                                       pCodecCtx->width,
                                                       pCodecCtx->height,
                                                       AV_PIX_FMT_BGR24,
                                                       SWS_BICUBIC,
                                                       Q_NULLPTR,
                                                       Q_NULLPTR,
                                                       Q_NULLPTR);
                sws_scale(img_convert_ctx,
                          ((AVPicture*)pFrame)->data,
                          ((AVPicture*)pFrame)->linesize,
                          0,
                          pCodecCtx->height,
                          ((AVPicture *)pFrameRGB)->data,
                          ((AVPicture *)pFrameRGB)->linesize);

                try {
                    qDebug() << "try to read result from ffmpeg" <<currentFrame;
                    Magick::Image image((size_t)pFrame->width,
                                        (size_t)pFrame->height,
                                        "BGR",
                                        Magick::CharPixel,
                                        pFrameRGB->data[0]);
                    image.write("/tmp/fofo.jpg");
                }
                catch(Magick::Error &error_ ) {
                    qWarning() << error_.what();
                }
                catch(Magick::Warning &warn_ ) {
                    qWarning() << warn_.what();
                }

                av_free_packet(&packet);
                sws_freeContext(img_convert_ctx);
                break;
            }
            currentFrame++;
        }
    }

    av_free_packet(&packet);
    avcodec_close(pCodecCtx);
    av_free(pFrame);
    av_free(pFrameRGB);
    avformat_close_input(&pFormatCtx);
}
