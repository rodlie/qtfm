#ifndef DFMQFILEITEMDELEGATE_H
#define DFMQFILEITEMDELEGATE_H

#include <QtWidgets>

// First column, index 0, is the Name column
#define COLUMN_NAME 0

/**
 * Extends QFileItemDelegate by a minimized selection way
 * for the name column of the details view.
 */
class DfmQStyledItemDelegate : public QStyledItemDelegate
{
public:
    explicit DfmQStyledItemDelegate(QObject* parent = 0);
    virtual ~DfmQStyledItemDelegate();

    /**
     * If minimized is true, the selection are
     * only drawn above the icon and text of an item.
     */
    void setMinimizedNameColumnSelection(bool minimized);
    bool hasMinimizedNameColumnSelection() const;

    virtual void paint(QPainter* painter,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    /**
     * Returns the minimized width of the name column for the name.
     */
    static int nameColumnWidth(const QString& name, const QStyleOptionViewItem& option);

private:
    bool m_hasMinimizedNameColumnSelection;
};

inline void DfmQStyledItemDelegate::setMinimizedNameColumnSelection(bool minimized)
{
    m_hasMinimizedNameColumnSelection = minimized;
}

inline bool DfmQStyledItemDelegate::hasMinimizedNameColumnSelection() const
{
    return m_hasMinimizedNameColumnSelection;
}

#endif // DFMQFILEITEMDELEGATE_H
