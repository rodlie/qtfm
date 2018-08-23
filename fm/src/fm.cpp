#include "fm.h"
#include <QVBoxLayout>

FM::FM(bool realMime,
       MimeUtils *mimeUtils,
        QWidget *parent) : QWidget(parent)
  , realMimeTypes(realMime)
  , mimeUtilsPtr(mimeUtils)
  , list(NULL)
  , modelTree(NULL)
  , modelView(NULL)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    /*modelTree = new mainTreeFilterProxyModel();
    modelTree->setSourceModel(modelList);
    modelTree->setSortCaseSensitivity(Qt::CaseInsensitive);

    modelView = new viewsSortProxyModel();
    modelView->setSourceModel(modelList);
    modelView->setSortCaseSensitivity(Qt::CaseInsensitive);*/

    list = new QListView(this);
    layout->addWidget(list);

    modelList = new myModel(realMimeTypes, mimeUtilsPtr);
    //connect(modelList, SIGNAL(reloadDir()), this, SLOT(dirLoaded()));

}
