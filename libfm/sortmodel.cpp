#include "sortmodel.h"
#include <QDir>
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
void viewsSortProxyModel::setNameFilter(const QString &text, bool filterDirs,
                                        const QString &rootPath)
{
    const bool active = !text.isEmpty();
    if (!active && !nameFilterActive) { return; } // nothing to change

    nameFilterActive = active;
    nameFilterDirs = filterDirs;
    nameFilterRoot = QDir::cleanPath(rootPath);
    if (active) {
        const bool wildcard = text.contains('*') || text.contains('?') || text.contains('[');
        const QString pattern = wildcard ?
                    QRegularExpression::wildcardToRegularExpression(
                        text, QRegularExpression::NonPathWildcardConversion) :
                    QRegularExpression::escape(text);
        nameFilter = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    }
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

    // name filter (folders only when requested), only for the items inside
    // the current folder: the model is a tree from "/" and the views show
    // the current folder as their root, so filtering out an ancestor folder
    // would remove the current folder itself from the view
    if (nameFilterActive && nameFilter.isValid()
        && (nameFilterDirs || !fileModel->isDir(index0))) {
        const QString parentPath = QDir::cleanPath(fileModel->filePath(sourceParent));
        const bool insideRoot = !parentPath.isEmpty() && !nameFilterRoot.isEmpty() &&
                (parentPath == nameFilterRoot ||
                 parentPath.startsWith(nameFilterRoot == "/" ? nameFilterRoot : nameFilterRoot + "/"));
        if (insideRoot && !nameFilter.match(fileModel->fileName(index0)).hasMatch()) {
            return false;
        }
    }

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
