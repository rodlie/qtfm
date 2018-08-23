#ifndef FM_H
#define FM_H

#include <QObject>
#include <QWidget>
#include <QListView>
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
                QWidget *parent = nullptr);
private:
    bool realMimeTypes;
    MimeUtils *mimeUtilsPtr;
    myModel *modelList;
    QListView *list;
    QSortFilterProxyModel *modelTree;
    QSortFilterProxyModel *modelView;
signals:

public slots:
};

#endif // FM_H
