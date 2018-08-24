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
                QString startPath = QDir::homePath(),
                QWidget *parent = NULL);
    ~FM();
private:
    bool realMimeTypes;
    MimeUtils *mimeUtilsPtr;
    myModel *modelList;
    QListView *list;
    QSortFilterProxyModel *modelView;
    IconViewDelegate *modelViewDelegate;
    QItemSelectionModel *listSelectionModel;
    int zoom;

signals:

public slots:
    void setPath(QString path);
    QString getPath();

private slots:
    void dirLoaded();
    void updateGrid();
};

#endif // FM_H
