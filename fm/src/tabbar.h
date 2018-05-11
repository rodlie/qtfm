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

#ifndef TABBAR_H
#define TABBAR_H

#include <QtGui>
#include <QTabBar>

class tabBar : public QTabBar
{
    Q_OBJECT

public:
    tabBar(QHash<QString,QIcon> *);
    int addNewTab(QString path,int type);
    void setIcon(int index);
    void mousePressEvent(QMouseEvent * event);
    void addHistory(QString);
    void remHistory();
    QStringList *getHistory(int);
    int getType(int index);
    void setType(int type);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

signals:
        void dragDropTab(const QMimeData * data, QString newPath, QStringList cutList);

public slots:
        void closeTab();

private:
        QHash<QString,QIcon> *folderIcons;
        QList<QStringList*> history;
        QList<int> viewType;

};

#endif // TABBAR_H
