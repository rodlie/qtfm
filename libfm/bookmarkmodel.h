/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
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

#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#define MEDIA_MODEL Qt::UserRole+4
#define MEDIA_PATH Qt::UserRole+5

#include <QStandardItemModel>
//#include <QHash>
#include <QIcon>
#include <QMimeData>
#include <QModelIndex>
#include <QStringList>

class bookmarkmodel : public QStandardItemModel
{
    Q_OBJECT

public:
    bookmarkmodel(/*QHash<QString, QIcon> **/);
    QStringList mimeTypes() const;
    bool dropMimeData(const QMimeData *data,
                      Qt::DropAction action,
                      int row,
                      int column,
                      const QModelIndex &parent);
    void addBookmark(QString name,
                     QString path,
                     QString isAuto,
                     QString icon,
                     QString mediaPath = QString(),
                     bool isMedia = false,
                     bool changed = true);

signals:
    void bookmarkPaste(const QMimeData *data,
                       QString newPath,
                       QStringList cutList,
                       bool link = false);
    void bookmarksChanged();

//private:
    //QHash<QString, QIcon> *folderIcons;
};

#endif // BOOKMARKMODEL_H
