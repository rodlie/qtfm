#include "thumbs.h"
#include <Magick++.h>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "common.h"

Thumbs::Thumbs(QObject *parent) : QObject(parent)
{
    Magick::InitializeMagick(NULL);
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
        QStringList videos = Common::videoFormats();
        QFileInfo fileinfo(filename);
        if (videos.contains(fileinfo.suffix(), Qt::CaseInsensitive)) {
            thumb.read(QString("%1[100]").arg(filename).toUtf8().data());
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
