#ifndef DFMQLISTVIEW_H
#define DFMQLISTVIEW_H

#include <QListView>

/**
 * Extends QListView by a selection based on the area really covered by the
 * icon and the name of each item (like DfmQTreeView does for the details
 * view), instead of the whole item rectangle:
 *
 * - in list mode QListView gives each item the width of the whole column,
 *   so clicking or starting a drag in the blank space right of a short name
 *   would select or drag the file;
 * - in icon mode the item rectangle is usually wider than the name.
 *
 * The blank part of an item is handled as empty viewport area, and the
 * elastic band only selects items whose icon or name it touches.
 */
class DfmQListView : public QListView
{
    Q_OBJECT
public:
    explicit DfmQListView(QWidget *parent = nullptr);
    virtual ~DfmQListView();

    QModelIndex indexAt(const QPoint &point) const override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) override;

private:
    /**
     * Returns the rectangle (viewport coordinates) covered by the icon and
     * the text of the item, as painted by its delegate.
     */
    QRect itemContentRect(const QModelIndex &index) const;
};

#endif // DFMQLISTVIEW_H
