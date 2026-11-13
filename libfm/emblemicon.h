#ifndef EMBLEMICON_H
#define EMBLEMICON_H

#include <QIcon>
#include <QIconEngine>

/**
 * Icon engine that draws a base icon with an emblem in its bottom right
 * corner, at whatever size the icon is requested (zoom levels, HiDPI).
 *
 * Used to mark symbolic links: a link emblem for links, and a different
 * emblem over a dimmed icon for broken links (target does not exist).
 */
class EmblemIconEngine : public QIconEngine
{
public:
    EmblemIconEngine(const QIcon &base, const QIcon &emblem, bool broken);

    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;

    /**
     * Returns base with the symbolic link emblem, or with the broken link
     * emblem (and dimmed) when broken is true. Icons are cached.
     */
    static QIcon withEmblem(const QIcon &base, bool broken);

private:
    // used when the icon theme has no suitable emblem
    void paintFallbackEmblem(QPainter *painter, const QRect &rect) const;

    QIcon m_base;
    QIcon m_emblem;
    bool m_broken;
};

#endif // EMBLEMICON_H
