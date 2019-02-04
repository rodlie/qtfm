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

#include "bookmarkmodel.h"
#include "common.h"

#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QDir>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QAbstractButton>

bookmarkmodel::bookmarkmodel(QHash<QString,
                             QIcon> *icons)
{
    folderIcons = icons;
}

void bookmarkmodel::addBookmark(QString name,
                                QString path,
                                QString isAuto,
                                QString icon,
                                QString mediaPath,
                                bool isMedia,
                                bool changed)
{
    if (path.isEmpty() && !isMedia) { //add seperator
        QStandardItem *item = new QStandardItem(QIcon::fromTheme(icon), "");
        item->setData(QBrush(QPixmap(":/fm/images/sep.png")),
                      Qt::BackgroundRole);
        QFlags<Qt::ItemFlag> flags = item->flags();
        flags ^= Qt::ItemIsEditable; //not editable
        item->setFlags(flags);
        item->setFont(QFont("sans", 8)); //force size to prevent 2 rows of background tiling
        this->appendRow(item);
        return;
    }

    QIcon theIcon = QIcon::fromTheme(icon,
                                     QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    if (icon.isEmpty()) {
        if (folderIcons->contains(name)) { theIcon = folderIcons->value(name); }
    }

    if (name.isEmpty()) name = QString("/");
    QStandardItem *item = new QStandardItem(theIcon, name);
    item->setData(path, BOOKMARK_PATH);
    item->setData(icon, BOOKMARK_ICON);
    item->setData(isAuto, BOOKMARKS_AUTO);
    item->setData(isMedia, MEDIA_MODEL);
    if (isMedia) { item->setData(mediaPath, MEDIA_PATH); }
    this->appendRow(item);
    if (changed) { emit bookmarksChanged(); }
}

QStringList bookmarkmodel::mimeTypes() const
{
    return QStringList() << "application/x-qstandarditemmodeldatalist" << "text/uri-list";
}

bool bookmarkmodel::dropMimeData(const QMimeData * data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex &parent)
{
    //moving its own items around
    if (data->hasFormat("application/x-qstandarditemmodeldatalist"))
    if (parent.column() == -1) {
        return QStandardItemModel::dropMimeData(data,
                                                action,
                                                row,
                                                column,
                                                parent);
    }

    QList<QUrl> files = data->urls();
    QStringList cutList;
    QString parentPath = parent.data(BOOKMARK_PATH).toString();

    // Holding ctrl is copy, holding shift is move, holding alt is ask
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    Common::DragMode mode = Common::getDefaultDragAndDrop();
    if (mods == Qt::ControlModifier) {
        mode = Common::getDADctrlMod();
    } else if (mods == Qt::ShiftModifier) {
        mode = Common::getDADshiftMod();
    } else if (mods == Qt::AltModifier) {
        mode = Common::getDADaltMod();
    }

    // check if src/dst differ
    QString extraDialogText;
    foreach(QUrl path, files) {
        if (parent.column() == -1) { continue; }
        QFileInfo file(path.toLocalFile());
        // get original path
        QStringList getOldPath = file.absoluteFilePath().split("/",
                                                               QString::SkipEmptyParts);
        QString oldPath;
        for (int i=0;i<getOldPath.size()-1;++i) {
            oldPath.append(QString("/%1").arg(getOldPath.at(i)));
        }
        QString oldDevice = Common::getDeviceForDir(oldPath);
        QString newDevice = Common::getDeviceForDir(parentPath);
        if (oldDevice != newDevice) {
            extraDialogText = QString(tr("Source and destination is on a different storage."));
            mode = Common::DM_UNKNOWN;
            break;
        }
    }

    // If drag mode is unknown then ask what to do
    if (mode == Common::DM_UNKNOWN) {
        QMessageBox box;
        box.setWindowTitle(tr("Select file action"));
        box.setWindowIcon(QIcon::fromTheme("qtfm",
                                           QIcon(":/fm/images/qtfm.png")));
        box.setIconPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(32, 32)));
        box.setText(QString("<h3>%1</h3>")
                    .arg(tr("What do you want to do?")));
        if (!extraDialogText.isEmpty()) {
            box.setText(QString("%1<p>%2</p>")
            .arg(box.text())
            .arg(extraDialogText));
        }
        QAbstractButton *move = box.addButton(tr("Move here"),
                                              QMessageBox::ActionRole);
        QAbstractButton *copy = box.addButton(tr("Copy here"),
                                              QMessageBox::ActionRole);
        QAbstractButton *canc = box.addButton(QMessageBox::Cancel);
        move->setIcon(QIcon::fromTheme("edit-cut"));
        copy->setIcon(QIcon::fromTheme("edit-copy"));
        canc->setIcon(QIcon::fromTheme("edit-delete"));
        box.exec();
        if (box.clickedButton() == move) {
            mode = Common::DM_MOVE;
        } else if (box.clickedButton() == copy) {
            mode = Common::DM_COPY;
        } else if (box.clickedButton() == canc) {
            return false;
        }
    }

    foreach(QUrl path, files) {
        QFileInfo file(path.toLocalFile());
        //drag to bookmark window, add a new bookmark
        if (parent.column() == -1) {
            if (file.isDir()) this->addBookmark(file.fileName(),
                                                file.filePath(),
                                                Q_NULLPTR,
                                                "");
            return false;
        } else {
            if (mode == Common::DM_MOVE) {
                if (file.absoluteDir() != parentPath) { cutList.append(file.filePath()); }
            }
        }
    }
    emit bookmarkPaste(data,
                       parentPath,
                       cutList);
    return false;
}
