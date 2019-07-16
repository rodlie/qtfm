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
    if (m_hasMinimizedNameColumnSelection && (index.column() == COLUMN_NAME)) {
        QStyleOptionViewItem opt(option);

        QString filename = index.data(Qt::DisplayRole).toString();
        if (index.isValid()) {
            const int width = nameColumnWidth(filename, opt);
            opt.rect.setWidth(width);
        }
        QStyledItemDelegate::paint(painter, opt, index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

int DfmQStyledItemDelegate::nameColumnWidth(const QString& name, const QStyleOptionViewItem& option)
{
    QFontMetrics fontMetrics(option.font);
    return option.decorationSize.width() + fontMetrics.width(name) + 16;
}
