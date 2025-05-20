#include "dfmqtreeview.h"

#include <QApplication>
#include <QEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

#include "dfmqstyleditemdelegate.h"

DfmQTreeView::DfmQTreeView(QWidget *parent) :
    QTreeView(parent),
    m_expandingTogglePressed(false),
    m_useDefaultIndexAt(true),
    m_ignoreScrollTo(false),
    m_dropRect(),
    m_band()
{
    setUniformRowHeights(true);
    m_fileItemDelegate = new DfmQStyledItemDelegate(this);
    m_fileItemDelegate->setMinimizedNameColumnSelection(false); // if true long filenames will overflow the column!
    this->setItemDelegate(m_fileItemDelegate);
}

DfmQTreeView::~DfmQTreeView()
{
}

bool DfmQTreeView::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::Polish:
        m_useDefaultIndexAt = false;
        break;
    default:
        break;
    }
    return QTreeView::event(event);
}

void DfmQTreeView::mousePressEvent(QMouseEvent* event)
{
    const QModelIndex current = currentIndex();
    QTreeView::mousePressEvent(event);

    m_expandingTogglePressed = isAboveExpandingToggle(event->pos());

    const QModelIndex index = indexAt(event->pos());
    const bool updateState = index.isValid() &&
                             (index.column() == COLUMN_NAME) &&
                             (event->button() == Qt::LeftButton);
    if (updateState) {
        setState(QAbstractItemView::DraggingState);
    }

    if (!index.isValid() || (index.column() != COLUMN_NAME)) {
        const Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
        if (!m_expandingTogglePressed && !(mod & Qt::ShiftModifier) && !(mod & Qt::ControlModifier)) {
            clearSelection();
        }

        // Restore the current index, other columns are handled as viewport area.
        // setCurrentIndex(...) implicitly calls scrollTo(...), which we want to ignore.
        m_ignoreScrollTo = true;
        selectionModel()->setCurrentIndex(current, QItemSelectionModel::Current);
        m_ignoreScrollTo = false;

        if ((event->button() == Qt::LeftButton) && !m_expandingTogglePressed) {
            // Inform Qt about what we are doing - otherwise it starts dragging items around!
            setState(DragSelectingState);
            m_band.show = true;
            // Incremental update data will not be useful - start from scratch.
            m_band.ignoreOldInfo = true;
            const QPoint scrollPos(horizontalScrollBar()->value(), verticalScrollBar()->value());
            m_band.origin = event->pos()  + scrollPos;
            m_band.destination = m_band.origin;
            m_band.originalSelection = selectionModel()->selection();
        }
    }
}

void DfmQTreeView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_expandingTogglePressed) {
        // Per default QTreeView starts either a selection or a drag operation when dragging
        // the expanding toggle button
        // Turn off this behavior to stay predictable:
        setState(QAbstractItemView::NoState);
        return;
    }

    if (m_band.show) {
        const QPoint mousePos = event->pos();
        const QModelIndex index = indexAt(mousePos);
        if (!index.isValid()) {
            // The destination of the selection rectangle is above the viewport. In this
            // case QTreeView does no selection at all, which is not the wanted behavior.
            // Select all items within the elastic band rectangle.
            updateElasticBandSelection();
        }

        // TODO: Enable QTreeView::mouseMoveEvent(event) again, as soon
        // as the Qt-issue #199631 has been fixed.
        // QTreeView::mouseMoveEvent(event);
        QAbstractItemView::mouseMoveEvent(event);
        updateElasticBand();
    } else {
        // TODO: Enable QTreeView::mouseMoveEvent(event) again, as soon
        // as the Qt-issue #199631 has been fixed.
        // QTreeView::mouseMoveEvent(event);
        QAbstractItemView::mouseMoveEvent(event);
    }
}

void DfmQTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_expandingTogglePressed) {
        const QModelIndex index = indexAt(event->pos());
        if (index.isValid() && (index.column() == COLUMN_NAME)) {
            QTreeView::mouseReleaseEvent(event);
        } else {
            // don't change the current index if the cursor is released
            // above any other column than the name column, as the other
            // columns act as viewport
            const QModelIndex current = currentIndex();
            QTreeView::mouseReleaseEvent(event);
            selectionModel()->setCurrentIndex(current, QItemSelectionModel::Current);
        }
    }
    m_expandingTogglePressed = false;

    if (m_band.show) {
        setState(NoState);
        updateElasticBand();
        m_band.show = false;
    }
}

void DfmQTreeView::paintEvent(QPaintEvent* event)
{   
    QTreeView::paintEvent(event);
    if (m_band.show) {
        // The following code has been taken from QListView and adapted
        // (C) 1992-2007 Trolltech ASA
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = elasticBandRect();

        QPainter painter(viewport());
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }
}

QModelIndex DfmQTreeView::indexAt(const QPoint& point) const
{
    // The blank portion of the name column counts as empty space
    const QModelIndex index = QTreeView::indexAt(point);
    bool isTheNameColumn = index.column() == COLUMN_NAME;
    if (!isTheNameColumn) return QModelIndex(); //only do selection on the name column.
    const bool isAboveEmptySpace  = !m_useDefaultIndexAt &&
                                    isTheNameColumn &&
                                    !nameColumnRect(index).contains(point);
    return isAboveEmptySpace ? QModelIndex() : index;
}

void DfmQTreeView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
{
    command &= ~QItemSelectionModel::Rows; //remove the QItemSelectionModel::Rows flag for selection only in the name column

    // We must override setSelection() as Qt calls it internally and when this happens
    // we must ensure that the default indexAt() is used.
    if (!m_band.show) {
        m_useDefaultIndexAt = true;
        QTreeView::setSelection(rect, command);
        m_useDefaultIndexAt = false;
    } else {
        // Use our own elastic band selection algorithm
        updateElasticBandSelection();
    }
}

void DfmQTreeView::scrollTo(const QModelIndex & index, ScrollHint hint)
{
    if (!m_ignoreScrollTo) {
        QTreeView::scrollTo(index, hint);
    }
}

void DfmQTreeView::updateElasticBandSelection()
{
    if (!m_band.show) {
        return;
    }

    // Ensure the elastic band itself is up-to-date, in
    // case we are being called due to e.g. a drag event.
    updateElasticBand();

    // Clip horizontally to the name column, as some filenames will be
    // longer than the column.  We don't clip vertically as origin
    // may be above or below the current viewport area.
    const int nameColumnX = header()->sectionPosition(COLUMN_NAME);
    const int nameColumnWidth = header()->sectionSize(COLUMN_NAME);
    QRect selRect = elasticBandRect().normalized();
    QRect nameColumnArea(nameColumnX, selRect.y(), nameColumnWidth, selRect.height());
    selRect = nameColumnArea.intersected(selRect).normalized();
    // Get the last elastic band rectangle, expressed in viewpoint coordinates.
    const QPoint scrollPos(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QRect oldSelRect = QRect(m_band.lastSelectionOrigin - scrollPos, m_band.lastSelectionDestination - scrollPos).normalized();

    if (selRect.isNull()) {
        selectionModel()->select(m_band.originalSelection, QItemSelectionModel::ClearAndSelect);
        m_band.ignoreOldInfo = true;
        return;
    }

    if (!m_band.ignoreOldInfo) {
        // Do some quick checks to see if we can rule out the need to
        // update the selection.
        Q_ASSERT(uniformRowHeights());
        QModelIndex dummyIndex = model()->index(0, 0);
        if (!dummyIndex.isValid()) {
            // No items in the model presumably.
            return;
        }

        // If the elastic band does not cover the same rows as before, we'll
        // need to re-check, and also invalidate the old item distances.
        const int rowHeight = QTreeView::indexRowSizeHint(dummyIndex);
        const bool coveringSameRows =
            (selRect.top()    / rowHeight == oldSelRect.top()    / rowHeight) &&
            (selRect.bottom() / rowHeight == oldSelRect.bottom() / rowHeight);
        if (coveringSameRows) {
            // Covering the same rows, but have we moved far enough horizontally
            // that we might have (de)selected some other items?
            const bool itemSelectionChanged =
                ((selRect.left() > oldSelRect.left()) &&
                 (selRect.left() > m_band.insideNearestLeftEdge)) ||
                ((selRect.left() < oldSelRect.left()) &&
                 (selRect.left() <= m_band.outsideNearestLeftEdge)) ||
                ((selRect.right() < oldSelRect.right()) &&
                 (selRect.left() >= m_band.insideNearestRightEdge)) ||
                ((selRect.right() > oldSelRect.right()) &&
                 (selRect.right() >= m_band.outsideNearestRightEdge));

            if (!itemSelectionChanged) {
                return;
            }
        }
    } else {
        // This is the only piece of optimization data that needs to be explicitly
        // discarded.
        m_band.lastSelectionOrigin = QPoint();
        m_band.lastSelectionDestination = QPoint();
        oldSelRect = selRect;
    }

    // Do the selection from scratch. Force a update of the horizontal distances info.
    m_band.insideNearestLeftEdge   = nameColumnX + nameColumnWidth + 1;
    m_band.insideNearestRightEdge  = nameColumnX - 1;
    m_band.outsideNearestLeftEdge  = nameColumnX - 1;
    m_band.outsideNearestRightEdge = nameColumnX + nameColumnWidth + 1;

    // Include the old selection rect as well, so we can deselect
    // items that were inside it but not in the new selRect.
    const QRect boundingRect = selRect.united(oldSelRect).normalized();
    if (boundingRect.isNull()) {
        return;
    }

    // Get the index of the item in this row in the name column.
    QModelIndex startIndex = QTreeView::indexAt(boundingRect.topLeft());    
    int nameColumn = model()->headerData(COLUMN_NAME, Qt::Horizontal, Qt::DisplayRole).toInt();
    if (nameColumn != -1) {
        if (startIndex.parent().isValid()) {
            startIndex = model()->index(startIndex.row(), nameColumn, startIndex.parent());
        } else {
            startIndex = model()->index(startIndex.row(), nameColumn);
        }
    }
    if (!startIndex.isValid()) {
        selectionModel()->select(m_band.originalSelection, QItemSelectionModel::ClearAndSelect);
        m_band.ignoreOldInfo = true;
        return;
    }

   // Go through all indexes between the top and bottom of boundingRect, and
   // update the selection.
   const int verticalCutoff = boundingRect.bottom();
   QModelIndex currIndex = startIndex;
   QModelIndex lastIndex;
   bool allItemsInBoundDone = false;

   // Calling selectionModel()->select(...) for each item that needs to be
   // toggled is slow as each call emits selectionChanged(...) so store them
   // and do the selection toggle in one batch.
   QItemSelection itemsToToggle;
   // QItemSelection's deal with continuous ranges of indexes better than
   // single indexes, so try to portion items that need to be toggled into ranges.
   bool formingToggleIndexRange = false;
   QModelIndex toggleIndexRangeBegin = QModelIndex();

   do {
       QRect currIndexRect = nameColumnRect(currIndex);

        // Update some optimization info as we go.
       const int cr = currIndexRect.right();
       const int cl = currIndexRect.left();
       const int sl = selRect.left();
       const int sr = selRect.right();
       // "The right edge of the name is outside of the rect but nearer than m_outsideNearestLeft", etc
       if ((cr < sl && cr > m_band.outsideNearestLeftEdge)) {
           m_band.outsideNearestLeftEdge = cr;
       }
       if ((cl > sr && cl < m_band.outsideNearestRightEdge)) {
           m_band.outsideNearestRightEdge = cl;
       }
       if ((cl >= sl && cl <= sr && cl > m_band.insideNearestRightEdge)) {
           m_band.insideNearestRightEdge = cl;
       }
       if ((cr >= sl && cr <= sr && cr < m_band.insideNearestLeftEdge)) {
           m_band.insideNearestLeftEdge = cr;
       }

       bool currentlySelected = selectionModel()->isSelected(currIndex);
       bool originallySelected = m_band.originalSelection.contains(currIndex);
       bool intersectsSelectedRect = currIndexRect.intersects(selRect);
       bool shouldBeSelected = (intersectsSelectedRect && !originallySelected) || (!intersectsSelectedRect && originallySelected);
       bool needToToggleItem = (currentlySelected && !shouldBeSelected) || (!currentlySelected && shouldBeSelected);
       if (needToToggleItem && !formingToggleIndexRange) {
            toggleIndexRangeBegin = currIndex;
            formingToggleIndexRange = true;
       }

       // NOTE: indexBelow actually walks up and down expanded trees for us.
       QModelIndex nextIndex = indexBelow(currIndex);
       allItemsInBoundDone = !nextIndex.isValid() || currIndexRect.top() > verticalCutoff;

       const bool commitToggleIndexRange = formingToggleIndexRange &&
                                           (!needToToggleItem ||
                                            allItemsInBoundDone ||
                                            currIndex.parent() != toggleIndexRangeBegin.parent());
       if (commitToggleIndexRange) {
           formingToggleIndexRange = false;
            // If this is the last item in the bounds and it is also the beginning of a range,
            // don't toggle lastIndex - it will already have been dealt with.
           if (!allItemsInBoundDone || toggleIndexRangeBegin != currIndex) {
               itemsToToggle.select(toggleIndexRangeBegin, lastIndex);
           }
            // Need to start a new range immediately with currIndex?
           if (needToToggleItem) {
               toggleIndexRangeBegin = currIndex;
               formingToggleIndexRange = true;
           }
           if (allItemsInBoundDone && needToToggleItem) {
                // Toggle the very last item in the bounds.
               itemsToToggle.select(currIndex, currIndex);
           }
       }

       // Next item
       lastIndex = currIndex;
       currIndex = nextIndex;
    } while (!allItemsInBoundDone);


    selectionModel()->select(itemsToToggle, QItemSelectionModel::Toggle);

    m_band.lastSelectionOrigin = m_band.origin;
    m_band.lastSelectionDestination = m_band.destination;
    m_band.ignoreOldInfo = false;
}

void DfmQTreeView::updateElasticBand()
{
    if (m_band.show) {
        QRect dirtyRegion(elasticBandRect());
        const QPoint scrollPos(horizontalScrollBar()->value(), verticalScrollBar()->value());
        m_band.destination = viewport()->mapFromGlobal(QCursor::pos()) + scrollPos;
        // Going above the (logical) top-left of the view causes complications during selection;
        // we may as well prevent it.
        if (m_band.destination.y() < 0) {
            m_band.destination.setY(0);
        }
        if (m_band.destination.x() < 0) {
            m_band.destination.setX(0);
        }
        dirtyRegion = dirtyRegion.united(elasticBandRect());
        setDirtyRegion(dirtyRegion);
    }
}

QRect DfmQTreeView::elasticBandRect() const
{
    const QPoint scrollPos(horizontalScrollBar()->value(), verticalScrollBar()->value());

    const QPoint topLeft = m_band.origin - scrollPos;
    const QPoint bottomRight = m_band.destination - scrollPos;
    return QRect(topLeft, bottomRight).normalized();
}

bool DfmQTreeView::isAboveExpandingToggle(const QPoint& pos) const
{
    // QTreeView offers no public API to get the information whether an index has an
    // expanding toggle and what boundaries the toggle has. The following approach
    // also assumes a toggle for file items.
    if (itemsExpandable()) {
        const QModelIndex index = QTreeView::indexAt(pos);
        if (index.isValid() && (index.column() == COLUMN_NAME)) {
            QRect rect = visualRect(index);
            const int toggleSize = rect.height();
            if (isRightToLeft()) {
                rect.moveRight(rect.right());
            } else {
                rect.moveLeft(rect.x() - toggleSize);
            }
            rect.setWidth(toggleSize);

            QStyleOption opt;
            opt.initFrom(this);
            opt.rect = rect;
            rect = style()->subElementRect(QStyle::SE_TreeViewDisclosureItem, &opt, this);

            return rect.contains(pos);
        }
    }
    return false;
}

QRect DfmQTreeView::nameColumnRect(const QModelIndex& index) const
{
    // The code guesses the width of the name
    QRect guessedItemContentRect = visualRect(index);

    if (index.isValid()) {
        QString filename = index.data(Qt::DisplayRole).toString();
        QStyleOptionViewItem option;
        option.initFrom(this);
        option.rect = guessedItemContentRect;
        const int itemContentWidth = DfmQStyledItemDelegate::nameColumnWidth(filename, option);
        guessedItemContentRect.setWidth(itemContentWidth);
    }

    return guessedItemContentRect;
}

DfmQTreeView::ElasticBand::ElasticBand() :
    show(false),
    origin(),
    destination(),
    lastSelectionOrigin(),
    lastSelectionDestination(),
    ignoreOldInfo(true),
    outsideNearestLeftEdge(0),
    outsideNearestRightEdge(0),
    insideNearestLeftEdge(0),
    insideNearestRightEdge(0)
{
}
