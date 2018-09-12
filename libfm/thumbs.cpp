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

#ifndef NO_FFMPEG
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}
#endif

Thumbs::Thumbs(QObject *parent) : QObject(parent)
  , abortProc(false)
{
    Magick::InitializeMagick(Q_NULLPTR);
#ifndef NO_FFMPEG
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();
#ifdef QT_NO_DEBUG
    av_log_set_level(AV_LOG_QUIET);
#endif
#endif
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
    if (ignoreList.contains(file)) {
        qDebug() << "ignore" << file;
        return;
    }
    if (abortProc) { abortProc = false; }
    QMetaObject::invokeMethod(this,"procIcon",
                              Q_ARG(QString, file),
                              Q_ARG(QString, mimetype));
}

void Thumbs::abort()
{
    qDebug() << "set abort flag";
    abortProc = true;
}


void Thumbs::procIcon(QString file, QString mimetype)
{
    if (!QFile::exists(file) || abortProc) { return; }
    if (ignoreList.contains(file)) {
        qDebug() << "ignore" << file;
        return;
    }
    qDebug() << "procIcon" << file << mimetype;

    QByteArray result;
    if (mimetype.startsWith("video/")) {
        result = getVideoFrame(file);
        if (result.length()>0) {
            emit generatedIcon(file, result);
        } else {
            ignoreList.append(file);
        }
        return;
    }
    try {
        QString bgColor = "black";
        if (mimetype == "application/pdf") { bgColor = "white"; }
        Magick::Image background(Magick::Geometry(128, 128),
                                 Magick::Color(bgColor.toStdString()));
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

        if (mimetype.startsWith("audio/")) {
            QByteArray rawPix = getVideoFrame(file,
                                              true /* get embedded image if any */);
            if (rawPix.length()>0) {
                Magick::Blob tmp(rawPix.data(), (size_t)rawPix.length());
                thumb.read(tmp);
            } else {
                ignoreList.append(file);
                return;
            }
        } else { thumb.read(filename.toUtf8().data()); }

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
        background.composite(thumb, offsetX, offsetY,
                             MagickCore::OverCompositeOp);
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

QByteArray Thumbs::getVideoFrame(QString file, bool getEmbedded, int videoFrame, int pixSize)
{
    qDebug() << "getVideoFrame" << file << getEmbedded << videoFrame << pixSize;
    QByteArray result;
#ifndef NO_FFMPEG
    if (file.isEmpty() || abortProc) { return result; }

    AVCodecContext  *pCodecCtx;
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVCodec * pCodec;
    AVFrame *pFrame, *pFrameRGB;

    qDebug() << "open media file";
    if (avformat_open_input(&pFormatCtx,file.toUtf8().data(),
                            Q_NULLPTR,
                            Q_NULLPTR) != 0) { return result; }
    if (avformat_find_stream_info(pFormatCtx,
                                  Q_NULLPTR) < 0) { return result; }

    av_dump_format(pFormatCtx, 0, file.toUtf8().data(), 0);
    int videoStream = -1;

    qDebug() << "get video stream";
    for (int i=0; i < (int)pFormatCtx->nb_streams; i++) {
        if (abortProc) { return result; }
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) { return result; }

    qDebug() << "find decoder";
    pCodec =avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    pCodecCtx = avcodec_alloc_context3(Q_NULLPTR);
    if (pCodec == Q_NULLPTR || pCodecCtx == Q_NULLPTR) { return result; }
    if (avcodec_parameters_to_context(pCodecCtx,
                                      pFormatCtx->streams[videoStream]->codecpar)<0)
    { return result; }
    if (avcodec_open2(pCodecCtx,
                     pCodec,
                     Q_NULLPTR) < 0) { return result; }

    qDebug() << "check for embedded?" << getEmbedded;
    if (getEmbedded) {
        if (pFormatCtx->streams[videoStream]->disposition == AV_DISPOSITION_ATTACHED_PIC) {
            AVPacket pkt = pFormatCtx->streams[videoStream]->attached_pic;
            if (pkt.size>0) {
                QByteArray attachedPix = QByteArray(reinterpret_cast<const char*>(pkt.data),
                                                    pkt.size);
                if (attachedPix.length()>0) {
                    qDebug() << "got embedded picture";
                    avcodec_close(pCodecCtx);
                    avformat_close_input(&pFormatCtx);
                    return attachedPix;
                }
            }
        }
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return  result;
    }

    qDebug() << "setup frame";
    pFrame    = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    uint8_t *buffer;
    int numBytes;

    AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
    numBytes = av_image_get_buffer_size(pFormat,
                                        pCodecCtx->width,
                                        pCodecCtx->height,
                                        16);
    buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data,
                         pFrameRGB->linesize,
                         buffer,
                         pFormat,
                         pCodecCtx->width,
                         pCodecCtx->height,
                         1);

    qDebug() << "calculate frame to get";
    int res;
    int frameFinished;
    AVPacket packet;
    int currentFrame = 0;
    bool fetchFrame = false;
    double fps = av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
    double dur = static_cast<double>(pFormatCtx->duration)/AV_TIME_BASE;
    int maxFrame = qRound((dur*fps)/2);
    if (videoFrame>=0) { maxFrame = videoFrame; }

    qDebug() << "we need to get frame" << maxFrame;
    int64_t seekT = (int64_t(maxFrame) *
                     pFormatCtx->streams[videoStream]->r_frame_rate.den *
                     pFormatCtx->streams[videoStream]->time_base.den) /
                     (int64_t(pFormatCtx->streams[videoStream]->r_frame_rate.num) *
                     pFormatCtx->streams[videoStream]->time_base.num);
    if (av_seek_frame(pFormatCtx, videoStream, seekT, AVSEEK_FLAG_FRAME) >= 0) {
              av_init_packet(&packet);
              currentFrame = maxFrame;
    }
    while((res = av_read_frame(pFormatCtx,&packet)>=0)) {
        if (abortProc) { break; }
        qDebug() << "at current frame" << currentFrame;
        if (currentFrame>=maxFrame) { fetchFrame = true; }
        if (packet.stream_index == videoStream){
            if (!fetchFrame) {
                currentFrame++;
                continue;
            }

            qDebug() << "get frame" << currentFrame;
            int ret = avcodec_send_packet(pCodecCtx, &packet);
            if (ret<0) { continue; }
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if (ret>=0) { frameFinished = true; }
            else { continue; }

            if (frameFinished) {
                qDebug() << "extract image from frame" << currentFrame;
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
                          ((AVFrame*)pFrame)->data,
                          ((AVFrame*)pFrame)->linesize,
                          0,
                          pCodecCtx->height,
                          ((AVFrame*)pFrameRGB)->data,
                          ((AVFrame*)pFrameRGB)->linesize);

                qDebug() << "prepare thumbnail for" << file;
                try {
                    Magick::Image background(Magick::Geometry((size_t)pixSize,
                                                              (size_t)pixSize),
                                             Magick::Color("black"));
#ifdef MAGICK7
                    background.alpha(true);
#else
                    background.matte(true);
#endif
                    background.backgroundColor(background.pixelColor(0,0));
                    background.transparent(background.pixelColor(0,0));

                    Magick::Image thumb((size_t)pFrame->width,
                                        (size_t)pFrame->height,
                                        "BGR",
                                        Magick::CharPixel,
                                        pFrameRGB->data[0]);
                    thumb.scale(Magick::Geometry((size_t)pixSize,
                                                 (size_t)pixSize));
                    int offsetX = 0;
                    int offsetY = 0;
                    if (thumb.columns()<background.columns()) {
                        offsetX = (int)(background.columns()-thumb.columns())/2;
                    }
                    if (thumb.rows()<background.rows()) {
                        offsetY = (int)(background.rows()-thumb.rows())/2;
                    }
                    background.composite(thumb,
                                         offsetX,
                                         offsetY,
                                         MagickCore::OverCompositeOp);
                    background.magick("BMP");
                    Magick::Blob buffer;
                    background.write(&buffer);
                    result = QByteArray((char*)buffer.data(),
                                        (int)buffer.length());
                    qDebug() << "thumbnail is done" << file;
                }
                catch(Magick::Error &error_ ) {
                    qWarning() << error_.what();
                }
                catch(Magick::Warning &warn_ ) {
                    qWarning() << warn_.what();
                }

                av_packet_unref(&packet);
                sws_freeContext(img_convert_ctx);
                break;
            }
            currentFrame++;
        }
    }

    av_packet_unref(&packet);
    avcodec_close(pCodecCtx);
    av_free(pFrame);
    av_free(pFrameRGB);
    avformat_close_input(&pFormatCtx);
#else
    Q_UNUSED(file)
    Q_UNUSED(getEmbedded)
    Q_UNUSED(videoFrame)
    Q_UNUSED(pixSize)
#endif
    return result;
}
