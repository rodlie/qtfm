/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef ICONLIST_H
#define ICONLIST_H

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QPainter>

class IconListDelegate : public QItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    /**
     * Returns the area actually covered by the icon and the text of the
     * item, in the same coordinates as option.rect.
     */
    static QRect contentRect(const QStyleOptionViewItem &option,
                             const QModelIndex &index);
};

#endif // ICONLIST_H
