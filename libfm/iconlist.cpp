#include "iconlist.h"

QSize IconListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
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

void IconListDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
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
