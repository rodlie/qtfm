
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QRegularExpression>

//---------------------------------------------------------------------------------
// Subclass QSortFilterProxyModel and override 'filterAcceptsRow' to only show
// directories in tree and not files.
//---------------------------------------------------------------------------------
class mainTreeFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};


//---------------------------------------------------------------------------------
// Subclass QSortFilterProxyModel and override 'lessThan' for sorting in list/details views
//---------------------------------------------------------------------------------
class viewsSortProxyModel : public QSortFilterProxyModel
{
public:
    // show the ".." entry of the current folder (detail view only)
    void setShowDotDot(bool show);
    // show only the items whose name contains text, or matches it when it
    // is a wildcard pattern (*.pdf); folders too when filterDirs is true.
    // Only the items inside rootPath (the current folder) are filtered.
    // An empty text removes the filter.
    void setNameFilter(const QString &text, bool filterDirs, const QString &rootPath);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
private:
    bool showDotDot = false;
    bool nameFilterActive = false;
    bool nameFilterDirs = false;
    QRegularExpression nameFilter;
    QString nameFilterRoot;
};
