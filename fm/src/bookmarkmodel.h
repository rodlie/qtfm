/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
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
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/

#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#define MEDIA_MODEL Qt::UserRole+1
#define MEDIA_PATH Qt::UserRole+2

#include <QtGui>

//---------------------------------------------------------------------------------
class bookmarkmodel : public QStandardItemModel
{
    Q_OBJECT

public:
    bookmarkmodel(QHash<QString,QIcon> *);
    QStringList mimeTypes () const;
    bool dropMimeData(const QMimeData * data,Qt::DropAction action,int row,int column,const QModelIndex & parent);
    void addBookmark(QString name, QString path, QString isAuto, QString icon, QString mediaPath = QString(), bool isMedia = false);

signals:
    void bookmarkPaste(const QMimeData * data, QString newPath, QStringList cutList);

private:
    QHash<QString,QIcon> *folderIcons;
};


#endif // BOOKMARKMODEL_H
