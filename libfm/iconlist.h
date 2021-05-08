/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QPainterPath>

class IconListDelegate : public QItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};
