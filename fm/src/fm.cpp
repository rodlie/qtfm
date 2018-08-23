#include "fm.h"
#include <QVBoxLayout>

FM::FM(bool realMime,
       MimeUtils *mimeUtils, QSortFilterProxyModel *parentTree,
        QWidget *parent) : QWidget(parent)
  , realMimeTypes(realMime)
  , mimeUtilsPtr(mimeUtils)
  , modelList(NULL)
  , list(NULL)
  , modelTree(parentTree)
  , modelView(NULL)
  , ivdelegate(NULL)
  , ildelegate(NULL)
  , treeSelectionModel(NULL)
  , listSelectionModel(NULL)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    modelList = new myModel(realMimeTypes, mimeUtilsPtr);
    connect(modelList, SIGNAL(reloadDir()), this, SLOT(dirLoaded()));

    modelView = new viewsSortProxyModel();
    modelView->setSourceModel(modelList);
    modelView->setSortCaseSensitivity(Qt::CaseInsensitive);

    ivdelegate = new IconViewDelegate();

    int zoom = 48;
    list = new QListView(this);
    list->setWrapping(true);
    list->setWordWrap(true);
    list->setModel(modelView);
    list->setTextElideMode(Qt::ElideNone);
    list->setViewMode(QListView::IconMode);
    list->setItemDelegate(ivdelegate);
    list->setGridSize(QSize(zoom, zoom));
    list->setIconSize(QSize(zoom, zoom));
    list->setFlow(QListView::LeftToRight);
    list->setMouseTracking(true);
    list->setDragDropMode(QAbstractItemView::DragDrop);
    list->setDefaultDropAction(Qt::MoveAction);
    list->setResizeMode(QListView::Adjust);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list->setSelectionRectVisible(true);
    list->setFocus();
    list->setEditTriggers(QAbstractItemView::EditKeyPressed |
                          QAbstractItemView::SelectedClicked);

    listSelectionModel = list->selectionModel();

    layout->addWidget(list);
}

FM::~FM()
{
}

void FM::treeSelectionChanged(QModelIndex current, QModelIndex previous)
{
    qDebug() << "treeSelectionChanged";
    Q_UNUSED(previous)

    QFileInfo name = modelList->fileInfo(modelTree->mapToSource(current));
    qDebug() << "new path" << name.filePath();
    if (!name.exists()) { return; }
    currentPath = name.filePath();

    if (modelList->setRootPath(name.filePath())) { modelView->invalidate(); }
    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(name.filePath()));
    list->setRootIndex(baseIndex);

    listSelectionModel->blockSignals(1);
    listSelectionModel->clear();
    listSelectionModel->blockSignals(0);
}

void FM::dirLoaded()
{
    qDebug() << "dirLoaded";
    updateGrid();
}

void FM::updateGrid()
{
    qDebug() << "updateGrid";
}
