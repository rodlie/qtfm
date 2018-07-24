/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012Wittfella
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

#ifndef BOOKMARKS_CPP
#define BOOKMARKS_CPP

#include <QtGui>
#include <QStatusBar>
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include "bookmarkmodel.h"
#include "icondlg.h"
#include "mainwindow.h"
#include "common.h"

//---------------------------------------------------------------------------
void MainWindow::addBookmarkAction()
{
    modelBookmarks->addBookmark(curIndex.fileName(),curIndex.filePath(),"0","");
}

//---------------------------------------------------------------------------
void MainWindow::addSeparatorAction()
{
    modelBookmarks->addBookmark("","","","");
}

//---------------------------------------------------------------------------
bookmarkmodel::bookmarkmodel(QHash<QString, QIcon> *icons)
{
    folderIcons = icons;
}

//---------------------------------------------------------------------------
void bookmarkmodel::addBookmark(QString name, QString path, QString isAuto, QString icon, QString mediaPath, bool isMedia, bool changed)
{
    //qDebug() << "add bookmark" << name << path << isAuto << icon << mediaPath << isMedia;
    if(path.isEmpty() && !isMedia)	    //add seperator
    {
        QStandardItem *item = new QStandardItem(QIcon::fromTheme(icon),"");
        item->setData(QBrush(QPixmap(":/images/sep.png")),Qt::BackgroundRole);
        QFlags<Qt::ItemFlag> flags = item->flags();
        flags ^= Qt::ItemIsEditable;                    //not editable
        item->setFlags(flags);
        item->setFont(QFont("sans",8));                 //force size to prevent 2 rows of background tiling
        this->appendRow(item);
        return;
    }

    QIcon theIcon;
    theIcon = QIcon::fromTheme(icon,QApplication::style()->standardIcon(QStyle::SP_DirIcon));

    if(icon.isEmpty()) {
        if(folderIcons->contains(name)) { theIcon = folderIcons->value(name); }
    }

    if(name.isEmpty()) name = "/";
    QStandardItem *item = new QStandardItem(theIcon,name);
    item->setData(path, BOOKMARK_PATH);
    item->setData(icon, BOOKMARK_ICON);
    item->setData(isAuto, BOOKMARKS_AUTO);
    item->setData(isMedia, MEDIA_MODEL);
    if (isMedia) { item->setData(mediaPath, MEDIA_PATH); }
    this->appendRow(item);
    if (changed) { emit bookmarksChanged(); }
}

//---------------------------------------------------------------------------
void MainWindow::delBookmark()
{
    QModelIndexList list = bookmarksList->selectionModel()->selectedIndexes();

    while(!list.isEmpty())
    {
        if(list.first().data(BOOKMARKS_AUTO).toString() == "1")		//automount, add to dontShowList
        {
            QStringList temp = settings->value("hideBookmarks",0).toStringList();
            temp.append(list.first().data(BOOKMARK_PATH).toString());
            settings->setValue("hideBookmarks",temp);
        }
        modelBookmarks->removeRow(list.first().row());
        list = bookmarksList->selectionModel()->selectedIndexes();
    }
    handleBookmarksChanged();
}

//---------------------------------------------------------------------------------
void MainWindow::editBookmark()
{
    icondlg * themeIcons = new icondlg;
    if(themeIcons->exec() == 1)
    {
        QStandardItem * item = modelBookmarks->itemFromIndex(bookmarksList->currentIndex());
        item->setData(themeIcons->result, BOOKMARK_ICON);
        item->setIcon(QIcon::fromTheme(themeIcons->result));
        handleBookmarksChanged();
    }
    delete themeIcons;
}

//---------------------------------------------------------------------------
void MainWindow::toggleWrapBookmarks()
{
    bookmarksList->setWrapping(wrapBookmarksAct->isChecked());
    settings->setValue("wrapBookmarks",wrapBookmarksAct->isChecked());
}

//---------------------------------------------------------------------------
void MainWindow::bookmarkPressed(QModelIndex current)
{
#ifndef NO_UDISKS
    if (current.data(MEDIA_MODEL).toBool() && !current.data(MEDIA_PATH).toString().isEmpty()) {
        if (current.data(BOOKMARK_PATH).toString().isEmpty()) {
            disks->devices[current.data(MEDIA_PATH).toString()]->mount();
        }
    }
#endif
    if(QApplication::mouseButtons() == Qt::MidButton)
        tabs->setCurrentIndex(addTab(current.data(BOOKMARK_PATH).toString()));
}

//---------------------------------------------------------------------------
void MainWindow::bookmarkClicked(QModelIndex item)
{
    if(item.data(BOOKMARK_PATH).toString() == pathEdit->currentText()) return;

#ifndef NO_UDISKS
    if (item.data(MEDIA_MODEL).toBool() && !item.data(MEDIA_PATH).toString().isEmpty()) {
        if (item.data(BOOKMARK_PATH).toString().isEmpty()) {
            disks->devices[item.data(MEDIA_PATH).toString()]->mount();
        }
    }
#endif

    QString info(item.data(BOOKMARK_PATH).toString());
    if(info.isEmpty()) return;                                  //separator
    if(info.contains("/.")) modelList->setRootPath(info);       //hidden folders

    tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(item.data(BOOKMARK_PATH).toString())));
    status->showMessage(getDriveInfo(curIndex.filePath()));
}

//---------------------------------------------------------------------------------
QStringList bookmarkmodel::mimeTypes() const
{
    return QStringList() << "application/x-qstandarditemmodeldatalist" << "text/uri-list";
}

//---------------------------------------------------------------------------------
bool bookmarkmodel::dropMimeData(const QMimeData * data,Qt::DropAction action,int row,int column,const QModelIndex & parent )
{
    //moving its own items around
    if(data->hasFormat("application/x-qstandarditemmodeldatalist"))
    if(parent.column() == -1) {
        return QStandardItemModel::dropMimeData(data,action,row,column,parent);
    }

    QList<QUrl> files = data->urls();
    QStringList cutList;
    QString parentPath = parent.data(BOOKMARK_PATH).toString();

    foreach(QUrl path, files) {
        QFileInfo file(path.toLocalFile());

        //drag to bookmark window, add a new bookmark
        if(parent.column() == -1) {
            if(file.isDir()) this->addBookmark(file.fileName(),file.filePath(),0,"");
            return false;
        } else {
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

            // get original path
            QStringList getOldPath = file.absoluteFilePath().split("/", QString::SkipEmptyParts);
            QString oldPath;
            for (int i=0;i<getOldPath.size()-1;++i) { oldPath.append(QString("/%1").arg(getOldPath.at(i))); }
            QString oldDevice = Common::getDeviceForDir(oldPath);
            QString newDevice = Common::getDeviceForDir(parentPath);

            QString extraText;
            if (oldDevice != newDevice) {
                extraText = QString(tr("Source and destination is on a different storage."));
                mode = Common::DM_UNKNOWN;
            }

            // If drag mode is unknown then ask what to do
            if (mode == Common::DM_UNKNOWN) {
              QMessageBox box;
              box.setWindowTitle(tr("Select file action"));
              box.setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/images/qtfm.png")));
              box.setIconPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(32, 32)));
              box.setText(tr("<h3>What do you want to do?</h3>"));
              if (!extraText.isEmpty()) {
                  box.setText(QString("%1<p>%2</p>").arg(box.text()).arg(extraText));
              }
              QAbstractButton *move = box.addButton(tr("Move here"), QMessageBox::ActionRole);
              QAbstractButton *copy = box.addButton(tr("Copy here"), QMessageBox::ActionRole);
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
            if (mode == Common::DM_MOVE) { cutList.append(file.filePath()); }
        }
    }

    emit bookmarkPaste(data, parentPath, cutList);
    return false;
}

#endif


