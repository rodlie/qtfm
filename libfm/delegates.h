#ifndef DELEGATES_H
#define DELEGATES_H

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QKeyEvent>

class IconViewDelegate : public QStyledItemDelegate
{
private: // workaround for QTBUG
    mutable bool _isEditing;
    mutable QModelIndex _index;
protected: // workaround for QTBUG
    bool eventFilter(QObject * object, QEvent * event)
    {
        QWidget *editor = qobject_cast<QWidget*>(object);
        if(editor && event->type() == QEvent::KeyPress) {
            if(static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape){
                _isEditing = false;
                _index = QModelIndex();
            }
        }
        return QStyledItemDelegate::eventFilter(editor, event);
    }
public:
    void setEditorData(QWidget * editor, const QModelIndex & index) const
    { // workaround for QTBUG
        _isEditing = true;
        _index = index;
        QStyledItemDelegate::setEditorData(editor, index);
    }
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
    { // workaround for QTBUG
        QStyledItemDelegate::setModelData(editor, model, index);
        _isEditing = false;
        _index = QModelIndex();
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        int width = qMax(iconsize.width(), option.fontMetrics.averageCharWidth() * 14);
        QRect txtRect(0, 0, width, option.rect.height());
        QSize txtsize = option.fontMetrics.boundingRect(txtRect,
                                                        Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap|Qt::TextWrapAnywhere,
                                                        index.data().toString()).size();
        if (txtsize.width()>width) { width = txtsize.width(); }
        QSize size(width+8, txtsize.height()+iconsize.height()+8+8);
        return size;
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect iconRect(item.left()+(item.width()/2)-(iconsize.width()/2),
                       item.top()+4+4, iconsize.width(), iconsize.height());
        QRect txtRect(item.left()+4, item.top()+iconsize.height()+4+4+4,
                      item.width()-8, item.height()-iconsize.height()-4);
        QBrush txtBrush = qvariant_cast<QBrush>(index.data(Qt::ForegroundRole));
        bool isSelected = option.state & QStyle::State_Selected;
        bool isEditing = _isEditing && index==_index;

        /*QStyleOptionViewItem opt = option;
        initStyleOption(&opt,index);
        opt.decorationAlignment |= Qt::AlignCenter;
        opt.displayAlignment    |= Qt::AlignCenter;
        opt.decorationPosition   = QStyleOptionViewItem::Top;
        opt.features |= QStyleOptionViewItem::WrapText;
        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem,&opt,painter);*/

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);

        if (isSelected && !isEditing) {
            QPainterPath path;
            QRect frame(item.left(),item.top()+4, item.width(), item.height()-4);
            path.addRoundRect(frame, 15, 15);
            //  path.addRect(frame);
            painter->setOpacity(0.7);
            painter->fillPath(path, option.palette.highlight());
            painter->setOpacity(1.0);
        }

        painter->drawPixmap(iconRect, icon.pixmap(iconsize.width(),iconsize.height()));

        if (isEditing) { return; }
        if (isSelected) { painter->setPen(option.palette.highlightedText().color()); }
        else { painter->setPen(txtBrush.color()); }

        painter->drawText(txtRect,
                          Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap|Qt::TextWrapAnywhere,
                          index.data().toString());
    }
};

class IconListDelegate : public QItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect txtRect(item.left()+iconsize.width()+5,
                      item.top(), item.width(), item.height());
        QSize txtsize = option.fontMetrics.boundingRect(txtRect,
                                                        Qt::AlignLeft|Qt::AlignVCenter,
                                                        index.data().toString()).size();
        return QSize(txtsize.width()+iconsize.width()+10,iconsize.height());
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect iconRect(item.left(), item.top(), iconsize.width(), iconsize.height());
        QRect txtRect(item.left()+iconsize.width()+5,
                      item.top(), item.width()-iconsize.width()-5, item.height()-2);
        QBrush txtBrush = qvariant_cast<QBrush>(index.data(Qt::ForegroundRole));

        if (option.state & QStyle::State_Selected) {
            QPainterPath path;
            path.addRect(txtRect);
            painter->setOpacity(0.7);
            painter->fillPath(path, option.palette.highlight());
            painter->setOpacity(1.0);
        }

        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else { painter->setPen(txtBrush.color()); }

        painter->drawPixmap(iconRect, icon.pixmap(iconsize.width(),iconsize.height()));
        painter->drawText(txtRect, Qt::AlignLeft|Qt::AlignVCenter, index.data().toString());
    }
};

#endif // DELEGATES_H
