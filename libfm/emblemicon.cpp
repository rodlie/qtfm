#include "emblemicon.h"

#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

EmblemIconEngine::EmblemIconEngine(const QIcon &base, const QIcon &emblem, bool broken) :
    m_base(base),
    m_emblem(emblem),
    m_broken(broken)
{
}

void EmblemIconEngine::paint(QPainter *painter, const QRect &rect,
                             QIcon::Mode mode, QIcon::State state)
{
    painter->save();

    // Base icon, dimmed when the link is broken
    if (m_broken) { painter->setOpacity(0.5); }
    m_base.paint(painter, rect, Qt::AlignCenter, mode, state);
    painter->setOpacity(1.0);

    // Emblem in the bottom right corner
    const int side = qMax(8, qRound(qMin(rect.width(), rect.height()) * 0.45));
    const QRect emblemRect(rect.right() - side + 1, rect.bottom() - side + 1, side, side);
    if (!m_emblem.isNull()) {
        m_emblem.paint(painter, emblemRect, Qt::AlignCenter, mode, state);
    } else {
        paintFallbackEmblem(painter, emblemRect);
    }

    painter->restore();
}

void EmblemIconEngine::paintFallbackEmblem(QPainter *painter, const QRect &rect) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    // Light disc with a dark border, readable on light and dark backgrounds
    const QRectF disc = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    painter->setPen(QPen(QColor(40, 40, 40), qMax(1.0, rect.width() / 12.0)));
    painter->setBrush(QColor(245, 245, 245));
    painter->drawEllipse(disc);

    const qreal w = rect.width();
    const QPointF c = QRectF(rect).center();
    if (m_broken) {
        // red cross
        painter->setPen(QPen(QColor(200, 30, 30), qMax(1.5, w / 7.0), Qt::SolidLine, Qt::RoundCap));
        const qreal d = w * 0.22;
        painter->drawLine(QPointF(c.x() - d, c.y() - d), QPointF(c.x() + d, c.y() + d));
        painter->drawLine(QPointF(c.x() - d, c.y() + d), QPointF(c.x() + d, c.y() - d));
    } else {
        // curved arrow pointing up right
        painter->setPen(QPen(QColor(40, 40, 40), qMax(1.5, w / 8.0), Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        QPainterPath curve;
        curve.moveTo(c.x() - w * 0.25, c.y() + w * 0.22);
        curve.quadTo(c.x() - w * 0.22, c.y() - w * 0.15, c.x() + w * 0.18, c.y() - w * 0.15);
        painter->drawPath(curve);
        const QPointF tip(c.x() + w * 0.24, c.y() - w * 0.15);
        painter->drawLine(tip, QPointF(tip.x() - w * 0.14, tip.y() - w * 0.12));
        painter->drawLine(tip, QPointF(tip.x() - w * 0.14, tip.y() + w * 0.12));
    }
}

QPixmap EmblemIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    if (size.isEmpty()) { return pix; }
    QPainter painter(&pix);
    paint(&painter, QRect(QPoint(0, 0), size), mode, state);
    return pix;
}

QIconEngine *EmblemIconEngine::clone() const
{
    return new EmblemIconEngine(m_base, m_emblem, m_broken);
}

QIcon EmblemIconEngine::withEmblem(const QIcon &base, bool broken)
{
    // Most links share their icon with other files of the same type, so the
    // result is cached instead of being created for every repaint
    static QHash<quint64, QIcon> cache[2];
    QHash<quint64, QIcon> &icons = cache[broken ? 1 : 0];
    const quint64 key = base.cacheKey();
    const auto found = icons.constFind(key);
    if (found != icons.constEnd()) { return found.value(); }
    if (icons.size() > 2000) { icons.clear(); }

    QIcon emblem;
    if (broken) {
        emblem = QIcon::fromTheme("emblem-unreadable");
        if (emblem.isNull()) { emblem = QIcon::fromTheme("emblem-important"); }
    } else {
        emblem = QIcon::fromTheme("emblem-symbolic-link");
    }

    QIcon icon(new EmblemIconEngine(base, emblem, broken));
    icons.insert(key, icon);
    return icon;
}
