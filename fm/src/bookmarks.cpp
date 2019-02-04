/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
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
 */

#include "mainwindow.h"
#include "common.h"
#include "icondlg.h"

#include <QStatusBar>
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>

void MainWindow::addBookmarkAction()
{
    modelBookmarks->addBookmark(curIndex.fileName(),
                                curIndex.filePath(),
                                "0",
                                "");
}

void MainWindow::addSeparatorAction()
{
    modelBookmarks->addBookmark("",
                                "",
                                "",
                                "");
}

void MainWindow::delBookmark()
{
    QModelIndexList list = bookmarksList->selectionModel()->selectedIndexes();
    while (!list.isEmpty()) {
        if (list.first().data(BOOKMARKS_AUTO).toString() == "1") { //automount, add to dontShowList
            QStringList temp = settings->value("hideBookmarks", 0).toStringList();
            temp.append(list.first().data(BOOKMARK_PATH).toString());
            settings->setValue("hideBookmarks", temp);
        }
        modelBookmarks->removeRow(list.first().row());
        list = bookmarksList->selectionModel()->selectedIndexes();
    }
    handleBookmarksChanged();
}

void MainWindow::editBookmark()
{
    icondlg * themeIcons = new icondlg;
    if (themeIcons->exec() == 1) {
        QStandardItem * item = modelBookmarks->itemFromIndex(bookmarksList->currentIndex());
        item->setData(themeIcons->result,
                      BOOKMARK_ICON);
        item->setIcon(QIcon::fromTheme(themeIcons->result));
        handleBookmarksChanged();
    }
    delete themeIcons;
}

void MainWindow::toggleWrapBookmarks()
{
    bookmarksList->setWrapping(wrapBookmarksAct->isChecked());
    settings->setValue("wrapBookmarks",
                       wrapBookmarksAct->isChecked());
}

void MainWindow::bookmarkPressed(QModelIndex current)
{
#ifndef NO_UDISKS
    if (current.data(MEDIA_MODEL).toBool() &&
        !current.data(MEDIA_PATH).toString().isEmpty()) {
        if (current.data(BOOKMARK_PATH).toString().isEmpty()) {
            disks->devices[current.data(MEDIA_PATH).toString()]->mount();
        }
    }
#endif
    if (QApplication::mouseButtons() == Qt::MidButton) {
        tabs->setCurrentIndex(addTab(current.data(BOOKMARK_PATH).toString()));
    }
}

void MainWindow::bookmarkClicked(QModelIndex item)
{
    if (item.data(BOOKMARK_PATH).toString() == pathEdit->currentText()) { return; }

#ifndef NO_UDISKS
    if (item.data(MEDIA_MODEL).toBool() &&
        !item.data(MEDIA_PATH).toString().isEmpty()) {
        if (item.data(BOOKMARK_PATH).toString().isEmpty()) {
            disks->devices[item.data(MEDIA_PATH).toString()]->mount();
        }
    }
#endif

    QString info(item.data(BOOKMARK_PATH).toString());
    if (info.isEmpty()) { return; } //separator
    if (info.contains("/.")) { modelList->setRootPath(info); } //hidden folders

    tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(item
                                                                    .data(BOOKMARK_PATH)
                                                                    .toString())));
    status->showMessage(Common::getDriveInfo(curIndex.filePath()));
}
