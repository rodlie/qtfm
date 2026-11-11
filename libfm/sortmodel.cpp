#include "sortmodel.h"
#include "mymodel.h"

//---------------------------------------------------------------------------------
bool mainTreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceModel() == nullptr) { return false; }
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());
    if (fileModel == nullptr) { return false; }
    if (fileModel->isDotDot(index0)) { return false; } // only for the file views
    if (fileModel->isDir(index0)) {
        if (this->filterRegularExpression().pattern().isEmpty() || !fileModel->fileInfo(index0).isHidden()) { return true; }
    }

    return false;
}

//---------------------------------------------------------------------------------
void viewsSortProxyModel::setShowDotDot(bool show)
{
    if (show == showDotDot) { return; }
    showDotDot = show;
    invalidateFilter();
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());
    if (fileModel == nullptr) { return true; }

    // "..": shown on request, also when hidden files are not shown
    if (fileModel->isDotDot(index0)) { return showDotDot; }

    if (this->filterRegularExpression().pattern().isEmpty()) { return true; }

    if (fileModel->fileInfo(index0).isHidden()) { return false; }
    else { return true; }
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    myModel* fsModel = dynamic_cast<myModel*>(sourceModel());

    // ".." always comes first, whatever the sort order
    const bool leftDotDot = fsModel->isDotDot(left);
    const bool rightDotDot = fsModel->isDotDot(right);
    if (leftDotDot != rightDotDot) {
        return leftDotDot == (sortOrder() == Qt::AscendingOrder);
    }

    if ((fsModel->isDir(left) && !fsModel->isDir(right))) {
        return sortOrder() == Qt::AscendingOrder;
    } else if(!fsModel->isDir(left) && fsModel->isDir(right)) {
        return sortOrder() == Qt::DescendingOrder;
    }

    if(left.column() == 1) { // size
        if (fsModel->size(left) > fsModel->size(right)) { return true; }
        else { return false; }
    } else if (left.column() == 3) { // date
        if (fsModel->fileInfo(left).lastModified() > fsModel->fileInfo(right).lastModified()) { return true; }
        else { return false; }
    }

    return QSortFilterProxyModel::lessThan(left,right);
}
