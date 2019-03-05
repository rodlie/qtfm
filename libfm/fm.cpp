/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "fm.h"
#include <QVBoxLayout>
#include <QApplication>

FM::FM(MimeUtils *mimeUtils,
       QString startPath,
       QWidget *parent) : QWidget(parent)
  , mimeUtilsPtr(mimeUtils)
  , modelList(Q_NULLPTR)
  , list(Q_NULLPTR)
  , modelView(Q_NULLPTR)
  , modelViewDelegate(Q_NULLPTR)
  , listSelectionModel(Q_NULLPTR)
  , zoom(48)
  , history(Q_NULLPTR)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    modelList = new myModel(true, mimeUtilsPtr);
    connect(modelList, SIGNAL(reloadDir(QString)), this, SLOT(dirLoaded()));

    modelView = new viewsSortProxyModel();
    modelView->setSourceModel(modelList);
    modelView->setSortCaseSensitivity(Qt::CaseInsensitive);

    modelViewDelegate = new IconViewDelegate();

    list = new QListView(this);
    list->setWrapping(true);
    list->setWordWrap(true);
    list->setModel(modelView);
    list->setTextElideMode(Qt::ElideNone);
    list->setViewMode(QListView::IconMode);
    list->setItemDelegate(modelViewDelegate);
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

    connect(list, SIGNAL(clicked(QModelIndex))
            ,this, SLOT(listDoubleClicked(QModelIndex)));

    listSelectionModel = list->selectionModel();

    layout->addWidget(list);

    history = new QStringList();
    setPath(startPath);
}

FM::~FM()
{
    delete history;
    qDebug() << "bye, bye!";
}

void FM::setPath(QString path)
{
    QFileInfo name(path);
    if (!name.exists() || path == getPath()) { return; }
    if (modelList->setRootPath(path)) { modelView->invalidate(); }
    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(path));
    list->setRootIndex(baseIndex);
    addHistory(path);
    emit newPath(path);
    emit newWindowTitle(path.split("/", QString::SkipEmptyParts).takeLast());
    updateGrid();
    //dirLoaded();
}

QString FM::getPath()
{
    return modelList->getRootPath();
}

QStringList *FM::getHistory()
{
    return history;
}


void FM::dirLoaded()
{
    // TODO
    qDebug() << "dirLoaded" << getPath();
    emit updatedDir(getPath());
}

void FM::updateGrid()
{
    qDebug() << "updateGrid";
    QFontMetrics fm = fontMetrics();
    int textWidth = fm.averageCharWidth() * 17;
    int realTextWidth = fm.averageCharWidth() * 14;
    int textHeight = fm.lineSpacing() * 3;
    QSize grid;
    grid.setWidth(qMax(zoom, textWidth));
    grid.setHeight(zoom+textHeight);

    QModelIndexList items;
    for (int x = 0; x < modelList->rowCount(modelList->index(getPath())); ++x) {
        items.append(modelList->index(x,0,modelList->index(getPath())));
    }
    foreach (QModelIndex theItem,items) {
        QString filename = modelList->fileName(theItem);
        QRect item(0,0,realTextWidth,grid.height());
        QSize txtsize = fm.boundingRect(item, Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap|Qt::TextWrapAnywhere, filename).size();
        int newHeight = txtsize.height()+zoom+5+8+4;
        if  (txtsize.width()>grid.width()) { grid.setWidth(txtsize.width()); }
        if (newHeight>grid.height()) { grid.setHeight(newHeight); }
    }
    if (list->gridSize() != grid) {
        list->setGridSize(grid);
    }
}

void FM::listDoubleClicked(QModelIndex current)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if (mods == Qt::ControlModifier || mods == Qt::ShiftModifier) {
        return;
    }
    QModelIndex index = modelView->mapToSource(current);
    if (modelList->isDir(index)) {
        qDebug() << "move further down the rabbit hole...";
        setPath(modelList->filePath(index));
    } else {
        qDebug() << "is file";
    }
}

void FM::addHistory(QString path)
{
    if (path.isEmpty()) { return; }
    qDebug() << "add to history" << path;
    history->insert(0, path);
}

void FM::remHistory()
{
    qDebug() << "remove from history";
    history->removeFirst();
}
