/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include <QObject>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>
#include <QPainterPath>

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
};
