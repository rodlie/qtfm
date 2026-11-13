#include "dfmqstyleditemdelegate.h"

#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QFontMetrics>

DfmQStyledItemDelegate::DfmQStyledItemDelegate(QObject* parent) :
    QStyledItemDelegate(parent),
    m_hasMinimizedNameColumnSelection(false)
{
}

DfmQStyledItemDelegate::~DfmQStyledItemDelegate()
{
}

void DfmQStyledItemDelegate::paint(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    if (!m_hasMinimizedNameColumnSelection) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt(option);
    if (index.column() == COLUMN_NAME) {
        // Draw (and highlight) only the icon and the name. Never grow beyond
        // the column, otherwise long names would overflow into the next one.
        if (index.isValid()) {
            const QString filename = index.data(Qt::DisplayRole).toString();
            const int width = nameColumnWidth(filename, opt);
            opt.rect.setWidth(qMin(width, option.rect.width()));
        }
    } else {
        // Only the icon and the name show the selection
        opt.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);
    }
    QStyledItemDelegate::paint(painter, opt, index);
}

int DfmQStyledItemDelegate::nameColumnWidth(const QString& name, const QStyleOptionViewItem& option)
{
    QFontMetrics fontMetrics(option.font);
    return option.decorationSize.width() + fontMetrics.horizontalAdvance(name) + 16;
}
