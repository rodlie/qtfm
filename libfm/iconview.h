/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef ICONVIEW_H
#define ICONVIEW_H

#include <QObject>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>

class IconViewDelegate : public QStyledItemDelegate
{
private: // workaround for QTBUG
    mutable bool _isEditing;
    mutable QModelIndex _index;
protected: // workaround for QTBUG
    bool eventFilter(QObject * object,
                     QEvent * event);
public:
    void setEditorData(QWidget * editor,
                       const QModelIndex & index) const;
    void setModelData(QWidget * editor,
                      QAbstractItemModel * model,
                      const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    /**
     * Returns the area actually covered by the icon and the text of the
     * item (plus a small margin), in the same coordinates as option.rect.
     */
    static QRect contentRect(const QStyleOptionViewItem &option,
                             const QModelIndex &index);
};

#endif // ICONVIEW_H
