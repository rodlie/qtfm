#ifndef FM_H
#define FM_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QItemSelectionModel>
#include <QDebug>
#include "mymodel.h"
#include "mymodelitem.h"
#include "mimeutils.h"
#include "delegates.h"
#include "sortfilter.h"

class FM : public QWidget
{
    Q_OBJECT
public:
    explicit FM(bool realMime,
                MimeUtils* mimeUtils,
                QSortFilterProxyModel *parentTree = NULL,
                QWidget *parent = NULL);
    ~FM();
private:
    bool realMimeTypes;
    MimeUtils *mimeUtilsPtr;
    myModel *modelList;
    QListView *list;
    QSortFilterProxyModel *modelTree;
    QSortFilterProxyModel *modelView;
    IconViewDelegate *ivdelegate;
    IconListDelegate *ildelegate;
    QItemSelectionModel *treeSelectionModel;
    QItemSelectionModel *listSelectionModel;
    QString currentPath;
signals:

public slots:
    void treeSelectionChanged(QModelIndex current, QModelIndex previous);
private slots:
    void dirLoaded();
    void updateGrid();
};

#endif // FM_H
