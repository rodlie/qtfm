/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/

#include <QtGui>
#include "tabbar.h"

//---------------------------------------------------------------------------
tabBar::tabBar(QHash<QString,QIcon> * icons)
{
    folderIcons = icons;
    setAcceptDrops(1);
    setMovable(1);

    // keep history and view type with their tab when tabs are reordered
    connect(this, &QTabBar::tabMoved, this, [this](int from, int to) {
        if (from < 0 || to < 0 || from >= history.size() || to >= history.size()) { return; }
        history.move(from, to);
        viewType.move(from, to);
        updateTabColors();
    });
    connect(this, &QTabBar::currentChanged, this, &tabBar::updateTabColors);
}

//---------------------------------------------------------------------------
/**
 * @brief True when the tab bar uses a dark palette (e.g. "Dark mode"), where
 * the style does not make the current tab stand out
 */
static bool isDarkPalette(const QPalette &palette)
{
    return palette.color(QPalette::Window).lightness() < 128;
}

//---------------------------------------------------------------------------
/**
 * @brief With a dark palette, the current tab keeps the normal text color
 * and the other tabs are dimmed; with a light palette, default colors
 */
void tabBar::updateTabColors()
{
    const bool dark = isDarkPalette(palette());
    for (int i = 0; i < count(); ++i) {
        QColor color; // invalid: default color of the style
        if (dark) {
            color = (i == currentIndex()) ? palette().color(QPalette::WindowText)
                                          : QColor(150, 150, 150);
        }
        setTabTextColor(i, color);
    }
}

//---------------------------------------------------------------------------
/**
 * @brief Paints the tabs, and with a dark palette a highlight bar on the
 * outer edge of the current tab
 */
void tabBar::paintEvent(QPaintEvent *event)
{
    QTabBar::paintEvent(event);
    if (!isDarkPalette(palette()) || currentIndex() < 0) { return; }

    const QRect tab = tabRect(currentIndex());
    if (tab.isEmpty()) { return; }
    const int thickness = 3;
    QRect bar(tab.left() + 1, tab.top(), tab.width() - 2, thickness);
    if (shape() == QTabBar::RoundedSouth || shape() == QTabBar::TriangularSouth) {
        bar.moveBottom(tab.bottom());
    }
    QPainter painter(this);
    painter.fillRect(bar, palette().color(QPalette::Highlight));
}

//---------------------------------------------------------------------------
void tabBar::mousePressEvent(QMouseEvent * event)
{
    //middle-click to close tab
    if(event->button() == Qt::MiddleButton)
    {
        int tab = tabAt(event->pos());
        if(tab != -1)
        {
            delete history.at(tab);
            history.removeAt(tab);
            viewType.removeAt(tab);
            this->removeTab(tab);
            if (this->count()==1) { this->hide(); }
        }
    }
    else
    if(event->button() == Qt::RightButton)
        this->setCurrentIndex(tabAt(event->pos()));

    return QTabBar::mousePressEvent(event);
}

//---------------------------------------------------------------------------
void tabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

//---------------------------------------------------------------------------
void tabBar::dragMoveEvent(QDragMoveEvent *event)
{
    this->setCurrentIndex(tabAt(event->position().toPoint()));
    event->acceptProposedAction();
}

//---------------------------------------------------------------------------
void tabBar::dropEvent(QDropEvent *event)
{
    QList<QUrl> paths = event->mimeData()->urls();
    if (paths.isEmpty()) { event->ignore(); return; }
    QFileInfo file = QFileInfo(paths.at(0).toLocalFile());

    if(tabAt(event->position().toPoint()) == -1 && file.isDir())           //new tab
        addNewTab(file.filePath(),0);
    else
    {
        QStringList cutList;

        //don't do anything if you drag and drop in same folder
        if(file.canonicalPath() == tabData(currentIndex()).toString())
        {
            event->ignore();
            return;
        }

        if (event->proposedAction() == 2) { //cut, holding ctrl to copy is action 1
            foreach (QUrl item, paths) { cutList.append(item.toLocalFile()); }
        }

        emit dragDropTab(event->mimeData(), tabData(currentIndex()).toString(), cutList);
    }

    event->acceptProposedAction();
}

//---------------------------------------------------------------------------
int tabBar::addNewTab(QString path, int type)
{
    QFileInfo file(path);

    history.append(new QStringList(path));
    viewType.append(type);

    QString filename = file.fileName();
    if (filename.isEmpty()) { filename = "/"; }

    int newtab = addTab(filename);
    setTabData(newtab,file.filePath());
    setIcon(newtab);
    updateTabColors();
    if (this->count()>1 && this->isHidden()) { this->show(); }
    return newtab;
}

//---------------------------------------------------------------------------
void tabBar::setIcon(int index)
{
    if(folderIcons->contains(tabText(index))) setTabIcon(index,folderIcons->value(tabText(index)));
    else setTabIcon(index,QIcon::fromTheme("folder"));
}

//---------------------------------------------------------------------------
void tabBar::addHistory(QString path)
{
    history.at(currentIndex())->insert(0,path);
}

//---------------------------------------------------------------------------
void tabBar::remHistory()
{
    QStringList *current = history.value(currentIndex());
    if (current && !current->isEmpty()) { current->removeFirst(); }
}

//---------------------------------------------------------------------------
QStringList *tabBar::getHistory(int index)
{
    return history.value(index);
}

//---------------------------------------------------------------------------
int tabBar::getType(int index)
{
    return viewType.at(index);
}

//---------------------------------------------------------------------------
void tabBar::setType(int type)
{
    viewType.removeAt(currentIndex());
    viewType.insert(currentIndex(),type);
}

//---------------------------------------------------------------------------
void tabBar::closeTab()
{
    if (currentIndex()>-1) {
        delete history.at(currentIndex());
        history.removeAt(currentIndex());
        viewType.removeAt(this->currentIndex());
        removeTab(this->currentIndex());
    }
    if (this->count()==1) { this->hide(); }
}
