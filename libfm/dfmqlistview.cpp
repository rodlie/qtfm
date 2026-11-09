#include "dfmqlistview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QItemSelection>
#include <QStyleOptionViewItem>

#include "iconview.h"
#include "iconlist.h"

DfmQListView::DfmQListView(QWidget *parent) :
    QListView(parent)
{
}

DfmQListView::~DfmQListView()
{
}

QRect DfmQListView::itemContentRect(const QModelIndex &index) const
{
    const QRect rect = visualRect(index);
    if (!index.isValid()) { return rect; }

    QStyleOptionViewItem option;
    initViewItemOption(&option);
    option.rect = rect;

    QAbstractItemDelegate *delegate = itemDelegateForIndex(index);
    if (dynamic_cast<IconViewDelegate*>(delegate)) {
        return IconViewDelegate::contentRect(option, index);
    }
    if (dynamic_cast<IconListDelegate*>(delegate)) {
        return IconListDelegate::contentRect(option, index);
    }
    return rect;
}

QModelIndex DfmQListView::indexAt(const QPoint &point) const
{
    // The blank part of the item counts as empty space
    const QModelIndex index = QListView::indexAt(point);
    if (index.isValid() && !itemContentRect(index).contains(point)) {
        return QModelIndex();
    }
    return index;
}

void DfmQListView::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);

    const QModelIndex index = indexAt(event->position().toPoint());
    if (!index.isValid()) {
        const Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if (!(mod & Qt::ShiftModifier) && !(mod & Qt::ControlModifier)) {
            clearSelection();
        }
    }
}

void DfmQListView::setSelection(const QRect &rect,
                                QItemSelectionModel::SelectionFlags command)
{
    // Clicks and keyboard selection keep the default (logical) behavior
    if (state() != DragSelectingState || !model() || !selectionModel()) {
        QListView::setSelection(rect, command);
        return;
    }

    // Elastic band: select only the items whose icon or name intersects it
    const QRect band = rect.normalized();
    const QModelIndex root = rootIndex();
    QItemSelection selection;
    const int rows = model()->rowCount(root);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = model()->index(row, modelColumn(), root);
        if (!index.isValid() || isIndexHidden(index)) { continue; }
        const Qt::ItemFlags flags = model()->flags(index);
        if (!(flags & Qt::ItemIsSelectable) || !(flags & Qt::ItemIsEnabled)) { continue; }
        if (!visualRect(index).intersects(band)) { continue; }
        if (itemContentRect(index).intersects(band)) {
            selection.select(index, index);
        }
    }
    selectionModel()->select(selection, command);
}
