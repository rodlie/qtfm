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

#ifndef THUMBNAILS_H
#define THUMBNAILS_H

#include <QObject>
#include <QCryptographicHash>

class Thumbnail : public QObject
{
    Q_OBJECT

public:
    enum ThumbnailSize
    {
        ThumbnailSizeNormal /* 128x128 */,
        ThumbnailSizeLarge /* 256x256 */,
        ThumbnailSizeXLarge /* 512x512 */,
        ThumbnailSizeXXLarge /* 1024x1024 */
    };
    explicit Thumbnail(const QString &filename,
                       QCryptographicHash::Algorithm hash,
                       int iconSize = ThumbnailSizeLarge,
                       QObject *parent = Q_NULLPTR);
    const QString getCachePath();
    const QString getIconPath();
    bool isValid();

private:
    QString _filename;
    QCryptographicHash::Algorithm _hash;
    int _iconSize;
    const QString getFileHash();
    bool _valid;
    void checkIcon();
    void generateIcon();
};

class Thumbnails : public QObject
{
    Q_OBJECT

public:
    explicit Thumbnails(QObject *parent = Q_NULLPTR);
    bool isBusy();
    void setEnabled(bool enabled);
    bool isEnabled();

signals:
    void foundIcon(const QString &filename,
                   const QString &icon);

public slots:
    void requestIcon(const QString &filename,
                     QCryptographicHash::Algorithm hash = QCryptographicHash::Md5,
                     int iconSize = Thumbnail::ThumbnailSizeLarge);
    void requestIcons(const QStringList &filenames,
                      QCryptographicHash::Algorithm hash = QCryptographicHash::Md5,
                      int iconSize = Thumbnail::ThumbnailSizeLarge);

private slots:
    void getIcon(const QString &filename,
                 QCryptographicHash::Algorithm hash = QCryptographicHash::Md5,
                 int iconSize = Thumbnail::ThumbnailSizeLarge);

private:
    bool _busy;
    bool _enabled;
};

#endif // THUMBNAILS_H
