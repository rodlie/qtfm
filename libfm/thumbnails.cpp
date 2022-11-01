/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2010-2021 QtFM developers (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "thumbnails.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QDateTime>

#include <Magick++.h>

Thumbnail::Thumbnail(const QString &filename,
                     QCryptographicHash::Algorithm hash,
                     int iconSize,
                     QObject *parent)
    : QObject(parent)
    , _filename(filename)
    , _hash(hash)
    , _iconSize(iconSize)
    , _valid(false)
{
    checkIcon();
}

const QString Thumbnail::getCachePath()
{
    QString result = qgetenv("XDG_CACHE_HOME");
    if (result.isEmpty()) {
        result = QString("%1/.cache").arg(QDir::homePath());
    }
    result.append("/thumbnails");
    switch (_iconSize) {
    case ThumbnailSizeNormal:
        result.append("/normal");
        break;
    case ThumbnailSizeLarge:
        result.append("/large");
        break;
    case ThumbnailSizeXLarge:
        result.append("/x-large");
        break;
    case ThumbnailSizeXXLarge:
        result.append("/xx-large");
        break;
    }
    if (!QFile::exists(result)) {
        QDir dir(result);
        if (!dir.mkpath(result)) { result.clear(); }
    }
    return result;
}

const QString Thumbnail::getIconPath()
{
    QString cache = getCachePath();
    QString hash = getFileHash();
    if (cache.isEmpty() || hash.isEmpty()) { return QString(); }
    return QString("%1/%2.png").arg(cache).arg(hash);
}

bool Thumbnail::isValid()
{
    return _valid;
}

const QString Thumbnail::getFileHash()
{
    if (!QFile::exists(_filename)) { return QString(); }
    return QString(QCryptographicHash::hash(QUrl::fromLocalFile(_filename).toEncoded(), _hash).toHex());
}

void Thumbnail::checkIcon()
{
    qDebug() << "checkIcon";
    if (!QFile::exists(getIconPath())) {
        generateIcon();
        return;
    }
    Magick::Image thumb(getIconPath().toStdString());
    if (!thumb.isValid()) {
        _valid = false;
        return;
    }

    qint64 fileMtime = QFileInfo(_filename).lastModified().toSecsSinceEpoch();
    qint64 thumbMtime = QString::fromStdString(thumb.attribute(QString("Thumb::MTime").toStdString())).toUInt();
    if (fileMtime != thumbMtime) { generateIcon(); }
    else { _valid = true; }

    //qDebug() << "Thumb::URI" << QString::fromStdString(thumb.attribute(QString("Thumb::URI").toStdString()));
}

void Thumbnail::generateIcon()
{
    qDebug() << "generateIcon";
    qint64 fileMtime = QFileInfo(_filename).lastModified().toSecsSinceEpoch();
    QString fileUri = QUrl::fromUserInput(_filename).toString();
    QString icon = getIconPath();
    Magick::Image thumb;
    try {
#if MagickLibVersion >= 0x700
      thumb.alpha(true);
#else
      thumb.matte(true);
#endif
        switch (_iconSize) {
        case ThumbnailSizeNormal:
            thumb.size(Magick::Geometry(128, 128));
            break;
        case ThumbnailSizeLarge:
            thumb.size(Magick::Geometry(256, 256));
            break;
        case ThumbnailSizeXLarge:
            thumb.size(Magick::Geometry(512, 512));
            break;
        case ThumbnailSizeXXLarge:
            thumb.size(Magick::Geometry(1024, 1024));
            break;
        }
    }
    catch(Magick::Error &error_ ) {
        qWarning() << error_.what();
        _valid = false;
        return;
    }
    catch(Magick::Warning &warn_ ) { qDebug() << warn_.what(); }

    Magick::Image layer;
    try {
        layer.read(_filename.toStdString());
        layer.strip();
        layer.magick("MIFF");
    }
    catch(Magick::Error &error_ ) {
        qWarning() << error_.what();
        _valid = false;
        return;
    }
    catch(Magick::Warning &warn_ ) { qDebug() << warn_.what(); }
    if (!layer.isValid()) {
        qDebug() << "invalid image!";
        _valid = false;
        return;
    }
    try {
        layer.scale(Magick::Geometry(thumb.columns(), thumb.rows()));
        int offX = 0;
        int offY = 0;
        if (layer.columns() < thumb.columns()) {
            offX = (thumb.columns()/2) - (layer.columns()/2);
        }
        if (layer.rows() < thumb.rows()) {
            offY = (thumb.rows()/2) - (layer.rows()/2);
        }
        thumb.composite(layer, offX, offY, Magick::OverCompositeOp);
        thumb.depth(8);
        thumb.attribute(QString("Thumb::MTime").toStdString(), QString::number(fileMtime).toStdString());
        thumb.attribute(QString("Thumb::URI").toStdString(), fileUri.toStdString());
        thumb.attribute(QString("Software").toStdString(), QString("QtFM").toStdString());
        thumb.write(getIconPath().toStdString());
    }
    catch(Magick::Error &error_ ) {
        qWarning() << error_.what();
        _valid = false;
        return;
    }
    catch(Magick::Warning &warn_ ) { qDebug() << warn_.what(); }
    _valid = true;
}

Thumbnails::Thumbnails(QObject *parent)
    : QObject(parent)
    , _busy(false)
    , _enabled(true)
{
    Magick::InitializeMagick(Q_NULLPTR);
}

bool Thumbnails::isBusy()
{
    return _busy;
}

void Thumbnails::setEnabled(bool enabled)
{
    _enabled = enabled;
}

bool Thumbnails::isEnabled()
{
    return _enabled;
}

void Thumbnails::requestIcon(const QString &filename,
                             QCryptographicHash::Algorithm hash,
                             int iconSize)
{
    qDebug() << "requestIcon" << filename << hash << iconSize;
    QtConcurrent::run(this,
                      &Thumbnails::getIcon,
                      filename,
                      hash,
                      iconSize);
}

void Thumbnails::requestIcons(const QStringList &filenames,
                              QCryptographicHash::Algorithm hash,
                              int iconSize)
{
    for (int i = 0; i < filenames.size(); ++i) {
        requestIcon(filenames.at(i), hash, iconSize);
    }
}

void Thumbnails::getIcon(const QString &filename,
                         QCryptographicHash::Algorithm hash,
                         int iconSize)
{
    if (!isEnabled()) { return; }
    _busy = true;
    qDebug() << "getIcon" << filename << hash << iconSize;
    Thumbnail thumb(filename, hash, iconSize);
    _busy = false;
    if (thumb.isValid()) { emit foundIcon(filename, thumb.getIconPath()); }
}

