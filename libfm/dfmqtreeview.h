#ifndef DFMTREEVIEW_H
#define DFMTREEVIEW_H

#include <QTreeView>

// First column, index 0, is the Name column
#define COLUMN_NAME 0

/**
 * Extends QTreeView by a custom selection
 */
class DfmQTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit DfmQTreeView(QWidget *parent = 0);
    virtual ~DfmQTreeView();

    virtual QModelIndex indexAt (const QPoint& point) const;
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);

protected:

    virtual bool event(QEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command);

private slots:
    /**
     * If the elastic band is currently shown, update the elastic band based on the
     * current mouse position and ensure that the selection is the set of items
     * intersecting it.
     */
    void updateElasticBandSelection();

    /**
     * Updates the destination from the elastic band to the
     * current mouse position and triggers an update.
     */
    void updateElasticBand();

    /**
     * Returns the rectangle for the elastic band dependent from the
     * origin, the current destination and the viewport position.
     */
    QRect elasticBandRect() const;

private:
    /**
     * Returns true, if pos is within the expanding toggle of a tree.
     */
    bool isAboveExpandingToggle(const QPoint& pos) const;

private:
    bool m_expandingTogglePressed;
    bool m_useDefaultIndexAt; // true, if QTreeView::indexAt() should be used
    bool m_ignoreScrollTo;    // true if calls to scrollTo(...) should do nothing.

    QRect m_dropRect;

    QRect nameColumnRect(const QModelIndex& index) const;

    struct ElasticBand
    {
        ElasticBand();

        // Elastic band origin and destination coordinates are relative to t
        // he origin of the view, not the viewport.
        bool show;
        QPoint origin;
        QPoint destination;

        // Optimization mechanisms for use with elastic band selection.
        // Basically, allow "incremental" updates to the selection based
        // on the last elastic band shape.
        QPoint lastSelectionOrigin;
        QPoint lastSelectionDestination;

        // If true, compute the set of selected elements from scratch (slower)
        bool ignoreOldInfo;

        // Edges of the filenames that are closest to the edges of oldSelectionRect.
        // Used to decide whether horizontal changes in the elastic band are likely
        // to require us to re-check which items are selected.
        int outsideNearestLeftEdge;
        int outsideNearestRightEdge;
        int insideNearestLeftEdge;
        int insideNearestRightEdge;
        // The set of items that were selected at the time this band was shown.
        // NOTE: Unless CTRL was pressed when the band was created, this is always empty.
        QItemSelection originalSelection;
    } m_band;

};

#endif // DFMTREEVIEW_H
