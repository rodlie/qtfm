/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef FM_H
#define FM_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QDebug>
#include "mymodel.h"
#include "mymodelitem.h"
#include "mimeutils.h"
#include "iconlist.h"
#include "iconview.h"
#include "sortmodel.h"
#include "completer.h"

class FM : public QWidget
{
    Q_OBJECT

public:
    explicit FM(MimeUtils* mimeUtils,
                QString startPath = QDir::homePath(),
                QWidget *parent = Q_NULLPTR);
    ~FM();

private:
    MimeUtils *mimeUtilsPtr;
    myModel *modelList;
    QListView *list;
    QSortFilterProxyModel *modelView;
    IconViewDelegate *modelViewDelegate;
    QItemSelectionModel *listSelectionModel;
    int zoom;
    QStringList* history;
    QCompleter *customComplete;

signals:
    void newWindowTitle(const QString &title);
    void updatedDir(const QString &dir);
    void newPath(const QString &path);
    void openFile(const QString &file);
    void previewFile(const QString &file);

public slots:
    void setPath(QString path);
    QString getPath();
    QStringList *getHistory();
    QCompleter *getCompleter();

private slots:
    void dirLoaded();
    void updateGrid();
    void listDoubleClicked(QModelIndex current);
    void listClicked(QModelIndex current);
    void addHistory(QString path);
    void remHistory();
};

#endif // FM_H
