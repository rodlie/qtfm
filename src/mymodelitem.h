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

#ifndef MYMODELITEM_H
#define MYMODELITEM_H

#include <QDir>
#include <QFileIconProvider>
#include <QDateTime>

#define SEPARATOR QString("/")



class myModelItem
{

public:

    myModelItem(const QFileInfo& fileInfo, myModelItem* parent);
    ~myModelItem();

    myModelItem* childAt(int position);
    myModelItem* parent();
    myModelItem* matchPath(const QStringList& path, int startIndex = 0);

    QList<myModelItem*> children();

    int childCount() const;
    int childNumber() const;
    bool hasChild(QString);

    QString absoluteFilePath() const;
    QString fileName() const;

    QFileInfo fileInfo() const;

    void refreshFileInfo();
    void addChild(myModelItem *child);
    void removeChild(myModelItem *child);
    void changeName(QString);
    void clearAll();

    bool walked;
    bool dirty;
    bool watched;
    QString mMimeType;
    QString mPermissions;

private:

    QList<myModelItem*> mChildren;
    myModelItem* mParent;
    QFileInfo mFileInfo;
    QString mAbsFilePath;

};


#endif // MYMODELITEM_H
