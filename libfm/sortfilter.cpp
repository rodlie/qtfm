#include "sortfilter.h"
#include "mymodel.h"

bool mainTreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceModel() == Q_NULLPTR) { return false; }
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());
    if (fileModel == Q_NULLPTR) { return false; }
    if (fileModel->isDir(index0)) {
        if (this->filterRegExp().isEmpty() ||
                fileModel->fileInfo(index0).isHidden() == 0) { return true; }
    }
    return false;
}

bool viewsSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (this->filterRegExp().isEmpty()) { return true; }
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());
    if (fileModel->fileInfo(index0).isHidden()) { return false; }
    else { return true; }
}

bool viewsSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    myModel* fsModel = dynamic_cast<myModel*>(sourceModel());
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

QString myCompleter::pathFromIndex(const QModelIndex &index) const
{
    if (!index.isValid()) { return QString(); }
    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = model()->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());
    list[0].clear() ; // the join below will provide the separator
    return list.join("/");
}

QStringList myCompleter::splitPath(const QString &path) const
{
    QStringList parts = path.split("/");
    parts[0] = "/";
    return parts;
}
