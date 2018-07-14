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

#include "mainwindow.h"
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include <QLineEdit>

void MainWindow::createActionIcons() {

  actionIcons = new QList<QIcon>;

  /*QFile icons(QDir::homePath() + QString("/.config/%16/icon.cache").arg(APP));
  icons.open(QIODevice::ReadOnly);
  QDataStream out(&icons);
  out >> *actionIcons;
  icons.close();*/

  if (actionIcons->count() < 29) {
    actionIcons->append(QIcon::fromTheme("folder-new",QIcon(":/images/folder-new.png")));
    actionIcons->append(QIcon::fromTheme("document-new",QIcon(":/images/document-new.png")));
    actionIcons->append(QIcon::fromTheme("edit-cut",QIcon(":/images/cut.png")));
    actionIcons->append(QIcon::fromTheme("edit-copy",QIcon(":/images/copy.png")));
    actionIcons->append(QIcon::fromTheme("edit-paste",QIcon(":/images/paste.png")));
    actionIcons->append(QIcon::fromTheme("go-up",QIcon(":/images/up.png")));
    actionIcons->append(QIcon::fromTheme("go-previous",QIcon(":/images/back.png")));
    actionIcons->append(QIcon::fromTheme("go-home",QIcon(":/images/home.png")));
    actionIcons->append(QIcon::fromTheme("view-list-details",QIcon(":/images/details.png")));
    actionIcons->append(QIcon::fromTheme("view-list-icons",QIcon(":/images/icons.png")));
    actionIcons->append(QIcon::fromTheme("folder-saved-search",QIcon(":/images/hidden.png")));
    actionIcons->append(QIcon::fromTheme("address-book-new",QIcon(":/images/bookmark.png")));
    actionIcons->append(QIcon::fromTheme("bookmark-new",QIcon(":/images/bookmark.png")));
    actionIcons->append(QIcon::fromTheme("edit-clear",QIcon(":/images/clear.png")));
    actionIcons->append(QIcon::fromTheme("edit-delete",QIcon(":/images/delete.png")));
    actionIcons->append(QIcon::fromTheme("preferences-system",QIcon(":/images/preferences.png")));
    actionIcons->append(QIcon::fromTheme("document-properties",QIcon(":/images/properties.png")));
    actionIcons->append(QIcon::fromTheme("utilities-terminal",QIcon(":/images/terminal.png")));
    actionIcons->append(QIcon::fromTheme("document-open",QIcon(":/images/document-open.png")));
    actionIcons->append(QIcon::fromTheme("view-refresh",QIcon(":/images/refresh.png")));
    actionIcons->append(QIcon::fromTheme("application-exit",QIcon(":/images/exit.png")));
    actionIcons->append(QIcon::fromTheme("lock",QIcon(":/images/lock.png")));
    actionIcons->append(QIcon::fromTheme("key_bindings",QIcon(":/images/key_bindings.png")));
    actionIcons->append(QIcon::fromTheme("zoom-in",QIcon(":/images/zoom-in.png")));
    actionIcons->append(QIcon::fromTheme("zoom-out",QIcon(":/images/zoom-out.png")));
    actionIcons->append(QIcon::fromTheme("window-close",QIcon(":/images/window-close.png")));
    actionIcons->append(QIcon::fromTheme("tab-new",QIcon(":/images/folder-new.png")));          //26
    actionIcons->append(QIcon::fromTheme("user-trash",QIcon(":/images/user-trash.png")));          //27
    actionIcons->append(QIcon::fromTheme("document-new",QIcon(":/images/document-new.png")));          //28
    /*icons.open(QIODevice::WriteOnly);
    QDataStream out(&icons);
    out << *actionIcons;
    icons.close();*/
  }
}
//---------------------------------------------------------------------------

void MainWindow::createActions() {
  createActionIcons();
  actionList = new QList<QAction*>;

  newDirAct = new QAction(tr("New folder"), this);
  newDirAct->setStatusTip(tr("Create a new folder"));
  connect(newDirAct, SIGNAL(triggered()), this, SLOT(newDir()));
  newDirAct->setIcon(actionIcons->at(0));
  actionList->append(newDirAct);

  newFileAct = new QAction(tr("New file"), this);
  newFileAct->setStatusTip(tr("Create a new file"));
  connect(newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));
  newFileAct->setIcon(actionIcons->at(1));
  actionList->append(newFileAct);

  newWinAct = new QAction(tr("New window"), this);
  connect(newWinAct, SIGNAL(triggered()), this, SLOT(newWindow()));
  newWinAct->setIcon(actionIcons->at(28));
  actionList->append(newWinAct);

  openTabAct = new QAction(tr("New tab"), this);
  openTabAct->setStatusTip(tr("Middle-click things to open tab"));
  connect(openTabAct, SIGNAL(triggered()), this, SLOT(openTab()));
  openTabAct->setIcon(actionIcons->at(26));
  actionList->append(openTabAct);

  closeTabAct = new QAction(tr("Close tab"), this);
  closeTabAct->setStatusTip(tr("Middle-click tabs to close"));
  connect(closeTabAct, SIGNAL(triggered()), tabs, SLOT(closeTab()));
  closeTabAct->setIcon(actionIcons->at(25));
  actionList->append(closeTabAct);

  tabsOnTopAct = new QAction(tr("Tabs on top"), this);
  tabsOnTopAct->setStatusTip(tr("Tabs on top"));
  tabsOnTopAct->setCheckable(true);
  tabsOnTopAct->setIcon(QIcon::fromTheme("tab-new"));
  connect(tabsOnTopAct, SIGNAL(triggered()), this, SLOT(tabsOnTop()));
  actionList->append(tabsOnTopAct);

  cutAct = new QAction(tr("Cut"), this);
  cutAct->setStatusTip(tr("Move the current file"));
  connect(cutAct, SIGNAL(triggered()), this, SLOT(cutFile()));
  cutAct->setIcon(actionIcons->at(2));
  actionList->append(cutAct);

  copyAct = new QAction(tr("Copy"), this);
  copyAct->setStatusTip(tr("Copy the current file"));
  connect(copyAct, SIGNAL(triggered()), this, SLOT(copyFile()));
  copyAct->setIcon(actionIcons->at(3));
  actionList->append(copyAct);

  pasteAct = new QAction(tr("Paste"), this);
  pasteAct->setStatusTip(tr("Paste the file here"));
  pasteAct->setEnabled(false);
  connect(pasteAct, SIGNAL(triggered()), this, SLOT(pasteClipboard()));
  pasteAct->setIcon(actionIcons->at(4));
  actionList->append(pasteAct);

  upAct = new QAction(tr("Up"),this);
  upAct->setStatusTip(tr("Go up one directory"));
  connect(upAct, SIGNAL(triggered()),this,SLOT(goUpDir()));
  upAct->setIcon(actionIcons->at(5));
  actionList->append(upAct);

  backAct = new QAction(tr("Back"),this);
  backAct->setStatusTip(tr("Go back one directory"));
  connect(backAct, SIGNAL(triggered()),this,SLOT(goBackDir()));
  actionIcons->append(newDirAct->icon());
  backAct->setIcon(actionIcons->at(6));
  actionList->append(backAct);

  homeAct = new QAction(tr("Home"),this);
  homeAct->setStatusTip(tr("Go to home directory"));
  connect(homeAct, SIGNAL(triggered()),this,SLOT(goHomeDir()));
  actionIcons->append(newDirAct->icon());
  homeAct->setIcon(actionIcons->at(7));
  actionList->append(homeAct);

  detailAct = new QAction(tr("Detail view"),this);
  detailAct->setStatusTip(tr("Toggle detailed list"));
  detailAct->setCheckable(true);
  connect(detailAct, SIGNAL(triggered()),this,SLOT(toggleDetails()));
  detailAct->setIcon(actionIcons->at(8));
  actionList->append(detailAct);

  iconAct = new QAction(tr("Icon view"),this);
  iconAct->setStatusTip(tr("Toggle icon view"));
  iconAct->setCheckable(true);
  connect(iconAct, SIGNAL(triggered()),this,SLOT(toggleIcons()));
  iconAct->setIcon(actionIcons->at(9));
  actionList->append(iconAct);

  sortNameAct = new QAction(tr("Name"), this);
  sortNameAct->setStatusTip(tr("Sort icons by name"));
  sortNameAct->setCheckable(true);

  sortDateAct = new QAction(tr("Date"), this);
  sortDateAct->setStatusTip(tr("Sort icons by date"));
  sortDateAct->setCheckable(true);

  sortSizeAct = new QAction(tr("Size"), this);
  sortSizeAct->setStatusTip(tr("Sort icons by size"));
  sortSizeAct->setCheckable(true);

  sortByActGrp = new QActionGroup(this);
  sortByActGrp->addAction(sortNameAct);
  sortByActGrp->addAction(sortDateAct);
  sortByActGrp->addAction(sortSizeAct);
  connect(sortByActGrp, SIGNAL(triggered(QAction*)), SLOT(toggleSortBy(QAction*)));

  sortAscAct = new QAction(tr("Ascending"), this);
  sortAscAct->setStatusTip(tr("Sort icons in ascending order"));
  sortAscAct->setCheckable(true);
  connect(sortAscAct, SIGNAL(triggered()), this, SLOT(toggleSortOrder()));
  actionList->append(sortAscAct);

  hiddenAct = new QAction(tr("Hidden files"), this);
  hiddenAct->setStatusTip(tr("Toggle hidden files"));
  hiddenAct->setCheckable(true);
  connect(hiddenAct, SIGNAL(triggered()), this, SLOT(toggleHidden()));
  hiddenAct->setIcon(actionIcons->at(10));
  actionList->append(hiddenAct);

  // TODO: create filter act that will use grep like filtering
  /*filterAct = new QAction(tr("Filter..."), this);
  filterAct->setStatusTip(tr("Filter current directory"));
  connect(hiddenAct, SIGNAL(triggered()), this, SLOT(toggleFilter()));
  hiddenAct->setIcon(actionIcons->at(10));
  actionList->append(filterAct);*/

  addBookmarkAct = new QAction(tr("Add bookmark"), this);
  addBookmarkAct->setStatusTip(tr("Add this folder to bookmarks"));
  connect(addBookmarkAct, SIGNAL(triggered()), this, SLOT(addBookmarkAction()));
  addBookmarkAct->setIcon(actionIcons->at(12));
  actionList->append(addBookmarkAct);

  addSeparatorAct = new QAction(tr("Add separator"),this);
  addSeparatorAct->setStatusTip(tr("Add separator to bookmarks list"));
  connect(addSeparatorAct, SIGNAL(triggered()),this,SLOT(addSeparatorAction()));
  actionList->append(addSeparatorAct);

  delBookmarkAct = new QAction(tr("Remove bookmark"),this);
  delBookmarkAct->setStatusTip(tr("Remove this bookmark"));
  connect(delBookmarkAct, SIGNAL(triggered()),this,SLOT(delBookmark()));
  delBookmarkAct->setIcon(actionIcons->at(13));
  actionList->append(delBookmarkAct);

  editBookmarkAct = new QAction(tr("Edit icon"),this);
  editBookmarkAct->setStatusTip(tr("Change bookmark icon"));
  connect(editBookmarkAct, SIGNAL(triggered()),this,SLOT(editBookmark()));
  editBookmarkAct->setIcon(actionIcons->at(15));
  actionList->append(editBookmarkAct);

  wrapBookmarksAct = new QAction(tr("Wrap bookmarks"),this);
  wrapBookmarksAct->setCheckable(true);
  connect(wrapBookmarksAct, SIGNAL(triggered()),this,SLOT(toggleWrapBookmarks()));
  actionList->append(wrapBookmarksAct);

  trashAct = new QAction(tr("Move to Trash"), this);
  trashAct->setStatusTip(tr("Move selected to trash"));
  connect(trashAct, SIGNAL(triggered(bool)), this, SLOT(trashFile()));
  trashAct->setIcon(actionIcons->at(27));
  actionList->append(trashAct);

  deleteAct = new QAction(tr("Delete"), this);
  deleteAct->setStatusTip(tr("Delete selected"));
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteFile()));
  deleteAct->setIcon(actionIcons->at(14));
  actionList->append(deleteAct);

  settingsAct = new QAction(tr("Settings..."), this);
  settingsAct->setStatusTip(tr("Edit custom actions"));
  connect(settingsAct, SIGNAL(triggered()), this, SLOT(showEditDialog()));
  settingsAct->setIcon(actionIcons->at(15));
  actionList->append(settingsAct);

  renameAct = new QAction(tr("Rename"), this);
  renameAct->setStatusTip(tr("Rename file"));
  renameAct->setIcon(QIcon::fromTheme("format-text-italic"));
  connect(renameAct, SIGNAL(triggered()),this, SLOT(renameFile()));
  actionList->append(renameAct);

  terminalAct = new QAction(tr("Terminal"), this);
  terminalAct->setStatusTip(tr("Open virtual terminal"));
  connect(terminalAct, SIGNAL(triggered()), this, SLOT(terminalRun()));
  terminalAct->setIcon(actionIcons->at(17));
  actionList->append(terminalAct);

  openAct = new QAction(tr("Open"), this);
  openAct->setStatusTip(tr("Open the file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
  openAct->setIcon(actionIcons->at(18));
  actionList->append(openAct);

  openFolderAct = new QAction(tr("Enter folder"), this);
  connect(openFolderAct, SIGNAL(triggered()), this, SLOT(openFolderAction()));
  actionList->append(openFolderAct);

  runAct = new QAction(tr("Run"), this);
  runAct->setStatusTip(tr("Run this program"));
  connect(runAct, SIGNAL(triggered()), this, SLOT(runFile()));
  runAct->setIcon(actionIcons->at(19));
  actionList->append(runAct);

  exitAct = new QAction(tr("Quit"), this);
  exitAct->setStatusTip(tr("Quit %1 and stop the daemon").arg(APP_NAME));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(exitAction()));
  exitAct->setIcon(actionIcons->at(20));
  actionList->append(exitAct);

  closeAct = new QAction(tr("Close"), this);
  closeAct->setStatusTip(tr("Close %1").arg(APP_NAME));
  connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));
  closeAct->setIcon(actionIcons->at(25));
  actionList->append(closeAct);

  thumbsAct = new QAction(tr("Show thumbs"), this);
  thumbsAct->setStatusTip(tr("View thumbnails for image files"));
  thumbsAct->setCheckable(true);
  thumbsAct->setIcon(QIcon::fromTheme("image-x-generic"));
  connect(thumbsAct, SIGNAL(triggered()), this, SLOT(toggleThumbs()));
  actionList->append(thumbsAct);

  folderPropertiesAct = new QAction(tr("Properties"), this);
  folderPropertiesAct->setStatusTip(tr("View properties of selected items"));
  connect(folderPropertiesAct, SIGNAL(triggered()), this, SLOT(folderPropertiesLauncher()));
  folderPropertiesAct->setIcon(actionIcons->at(16));
  actionList->append(folderPropertiesAct);

  lockLayoutAct = new QAction(tr("Lock layout"), this);
  lockLayoutAct->setCheckable(true);
  connect(lockLayoutAct, SIGNAL(triggered()), this, SLOT(toggleLockLayout()));
  lockLayoutAct->setIcon(actionIcons->at(21));
  actionList->append(lockLayoutAct);

  /*escapeAct = new QAction(tr("Cancel"), this);
  connect(escapeAct, SIGNAL(triggered()), this, SLOT(refresh()));
  actionList->append(escapeAct);*/

  zoomInAct = new QAction(tr("Zoom in"), this);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomInAction()));
  zoomInAct->setIcon(actionIcons->at(23));
  actionList->append(zoomInAct);

  zoomOutAct = new QAction(tr("Zoom out"), this);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOutAction()));
  zoomOutAct->setIcon(actionIcons->at(24));
  actionList->append(zoomOutAct);

  focusAddressAct = new QAction(tr("Focus address"), this);
  connect(focusAddressAct, SIGNAL(triggered()), this, SLOT(focusAction()));
  actionList->append(focusAddressAct);

  focusTreeAct = new QAction(tr("Focus tree"), this);
  connect(focusTreeAct, SIGNAL(triggered()), this, SLOT(focusAction()));
  actionList->append(focusTreeAct);

  focusBookmarksAct = new QAction(tr("Focus bookmarks"), this);
  connect(focusBookmarksAct, SIGNAL(triggered()), this, SLOT(focusAction()));
  actionList->append(focusBookmarksAct);

  focusListAct = new QAction(tr("Focus list"), this);
  connect(focusListAct, SIGNAL(triggered()), this, SLOT(focusAction()));
  actionList->append(focusListAct);

  aboutAct = new QAction(tr("About"), this);
  aboutAct->setIcon(QIcon::fromTheme("user-home"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(showAboutBox()));
  actionList->append(aboutAct);

#ifndef NO_UDISKS
  mediaUnmountAct = new QAction(tr("Safely Remove"), this);
  mediaUnmountAct->setIcon(QIcon::fromTheme("media-eject"));
  connect(mediaUnmountAct, SIGNAL(triggered(bool)), this, SLOT(handleMediaUnmount()));

  mediaEjectAct = new QAction(tr("Eject"), this);
  mediaEjectAct->setIcon(QIcon::fromTheme("media-eject"));
  connect(mediaEjectAct, SIGNAL(triggered(bool)), this, SLOT(handleMediaEject()));
#endif
  clearCacheAct = new QAction(tr("Clear cache"), this);
  clearCacheAct->setIcon(QIcon::fromTheme("edit-clear"));
  connect(clearCacheAct, SIGNAL(triggered()), this, SLOT(clearCache()));

  // We don't need the icon list anymore
  delete actionIcons;
}
//---------------------------------------------------------------------------

/**
 * @brief Reads shortcuts and registers actions
 */
void MainWindow::readShortcuts() {

  // Loads shortcuts
  QHash<QString, QString> shortcuts;
  settings->beginGroup("customShortcuts");
  QStringList keys = settings->childKeys();
  for (int i = 0; i < keys.count(); ++i) {
    QStringList temp(settings->value(keys.at(i)).toStringList());
    shortcuts.insert(temp.at(0),temp.at(1));
  }
  settings->endGroup();

  // Default shortcuts
  if (shortcuts.count() == 0) {
    shortcuts.insert(newWinAct->text(),"ctrl+n");
    shortcuts.insert(openTabAct->text(),"ctrl+t");
    shortcuts.insert(closeTabAct->text(),"ctrl+w");
    shortcuts.insert(cutAct->text(),"ctrl+x");
    shortcuts.insert(copyAct->text(),"ctrl+c");
    shortcuts.insert(pasteAct->text(),"ctrl+v");
    shortcuts.insert(upAct->text(),"alt+up");
    shortcuts.insert(backAct->text(),"backspace");
    //shortcuts.insert(homeAct->text(),"f1");
    shortcuts.insert(hiddenAct->text(),"ctrl+h");
    shortcuts.insert(trashAct->text(), "del");
    shortcuts.insert(deleteAct->text(),"shift+del");
    shortcuts.insert(terminalAct->text(),"f1");
    shortcuts.insert(exitAct->text(),"ctrl+q");
    shortcuts.insert(renameAct->text(),"f2");
    //shortcuts.insert(escapeAct->text(),"esc");
    shortcuts.insert(zoomOutAct->text(),"ctrl+-");
    shortcuts.insert(zoomInAct->text(),"ctrl++");
    shortcuts.insert(focusAddressAct->text(), "ctrl+l");
    shortcuts.insert(iconAct->text(), "f3");
    shortcuts.insert(detailAct->text(), "f4");

    settings->beginGroup("customShortcuts");
    QHashIterator<QString, QString> i(shortcuts);
    int count = 0;
    while (i.hasNext()) {
        i.next();
        QStringList action;
        action << i.key() << i.value();
        settings->setValue(QString(count), action);
        ++count;
    }
    settings->endGroup();
    settings->sync();
  }

  // Remove all bookmarks from actions
  foreach (QAction* a, bookmarkActionList) {
    actionList->removeOne(a);
    delete a;
  }
  bookmarkActionList.clear();

  // Register bookmarks as actions
  QList<QStandardItem*> tmp = modelBookmarks->findItems("*", Qt::MatchWildcard);
  foreach (QStandardItem *item, tmp) {
    if (!item->text().isEmpty()) {
      QAction *tempAction = new QAction(item->icon(), item->text(), this);
      connect(tempAction, SIGNAL(triggered()), SLOT(bookmarkShortcutTrigger()));
      bookmarkActionList.append(tempAction);
      actionList->append(tempAction);
    }
  }

  // Add all actions to MainWindow so they work when menu is hidden
  // NOTE: QWidget can handle situation when two pointers to the same action
  // instance are added and holds only one of them
  foreach (QAction* action, *actionList) {
    QString text = shortcuts.value(action->text());
    if (!text.isEmpty()) {
      action->setShortcut(QKeySequence::fromString(text));
      addAction(action);
    }
  }
}
//---------------------------------------------------------------------------

void MainWindow::bookmarkShortcutTrigger() {
  QAction* sc = qobject_cast<QAction*>(sender());
  QModelIndex index = modelBookmarks->findItems(sc->text()).first()->index();
  bookmarksList->clearSelection();
  bookmarksList->setCurrentIndex(index);
  bookmarkClicked(index);
}
//---------------------------------------------------------------------------

void MainWindow::createMenus() {

  // File menu
  // ----------------------------------------------------------------------
  QMenu *fileMenu = new QMenu(tr("File"));
  fileMenu->addAction(newDirAct);
  fileMenu->addAction(newFileAct);
  fileMenu->addSeparator();
  fileMenu->addAction(newWinAct);
  fileMenu->addAction(openTabAct);
  fileMenu->addSeparator();
  fileMenu->addAction(closeAct);
  fileMenu->addAction(exitAct);

  // Edit menu
  // ----------------------------------------------------------------------
  QMenu *editMenu = new QMenu(tr("Edit"));
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);
  editMenu->addAction(renameAct);
  editMenu->addAction(trashAct);
  editMenu->addAction(deleteAct);
  editMenu->addSeparator();
  editMenu->addAction(addBookmarkAct);
  editMenu->addSeparator();
  editMenu->addAction(clearCacheAct);
  editMenu->addSeparator();
  editMenu->addAction(settingsAct);

  // View menu
  // ----------------------------------------------------------------------
  QMenu *viewMenu = new QMenu(tr("View"));

  QMenu *autoMenu = createPopupMenu();
  autoMenu->addSeparator();
  autoMenu->addAction(lockLayoutAct);
  autoMenu->setTitle(tr("Layout"));
  viewMenu->addMenu(autoMenu);

  QMenu *sortMenu = new QMenu(tr("Sort By"));
  sortMenu->addAction(sortNameAct);
  sortMenu->addAction(sortDateAct);
  sortMenu->addAction(sortSizeAct);
  sortMenu->addSeparator();
  sortMenu->addAction(sortAscAct);
  viewMenu->addMenu(sortMenu);

  viewMenu->addSeparator();
  viewMenu->addAction(iconAct);
  viewMenu->addAction(detailAct);
  viewMenu->addAction(hiddenAct);
  viewMenu->addSeparator();
  viewMenu->addAction(tabsOnTopAct);
  viewMenu->addAction(thumbsAct);
  viewMenu->addSeparator();
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
  viewMenu->addSeparator();
  viewMenu->addAction(upAct);
  viewMenu->addAction(backAct);
  viewMenu->addAction(homeAct);

  // Help menu
  // ----------------------------------------------------------------------
  QMenu* helpMenu = new QMenu(tr("Help"));
  helpMenu->addAction(aboutAct);

  // Place all menus on menu bar
  // ----------------------------------------------------------------------
  QMenuBar *menuBar = new QMenuBar;
  menuBar->addMenu(fileMenu);
  menuBar->addMenu(editMenu);
  menuBar->addMenu(viewMenu);
  menuBar->addMenu(helpMenu);
  menuToolBar->addWidget(menuBar);
}
//---------------------------------------------------------------------------

void MainWindow::createToolBars() {
  menuToolBar = addToolBar(tr("Menu"));
  menuToolBar->setObjectName("Menu");
  addToolBarBreak();

  navToolBar = addToolBar(tr("Navigate"));
  navToolBar->setObjectName("Navigate");
  navToolBar->addAction(backAct);
  navToolBar->addAction(upAct);
  navToolBar->addAction(homeAct);

  addressToolBar = addToolBar(tr("Address"));
  addressToolBar->setObjectName("Address");
  addressToolBar->addWidget(pathEdit);
  addressToolBar->addAction(terminalAct);
}
//---------------------------------------------------------------------------

void MainWindow::zoomInAction()
{
    int zoomLevel;

    if(focusWidget() == tree) {
        (zoomTree == 64) ? zoomTree=64 : zoomTree+= 8;
        tree->setIconSize(QSize(zoomTree,zoomTree));
        zoomLevel = zoomTree;
    } else if (focusWidget() == bookmarksList) {
        (zoomBook == 64) ? zoomBook=64 : zoomBook+= 8;
        bookmarksList->setIconSize(QSize(zoomBook,zoomBook));
        zoomLevel = zoomBook;
    } else {
        if(stackWidget->currentIndex() == 0) {
            if(iconAct->isChecked()) {
                (zoom == 128) ? zoom=128 : zoom+= 8;
                zoomLevel = zoom;
            } else {
                (zoomList == 128) ? zoomList=128 : zoomList+= 8;
                zoomLevel = zoomList;
            }
            toggleIcons();
        }
        else {
            (zoomDetail == 64) ? zoomDetail=64 : zoomDetail+= 8;
            detailTree->setIconSize(QSize(zoomDetail,zoomDetail));
            zoomLevel = zoomDetail;
        }
    }

    status->showMessage(QString(tr("Zoom: %1")).arg(zoomLevel));
    updateGrid();
}

//---------------------------------------------------------------------------
void MainWindow::zoomOutAction()
{
    int zoomLevel;

    if(focusWidget() == tree) {
        (zoomTree == 16) ? zoomTree=16 : zoomTree-= 8;
        tree->setIconSize(QSize(zoomTree,zoomTree));
        zoomLevel = zoomTree;
    } else if(focusWidget() == bookmarksList) {
        (zoomBook == 16) ? zoomBook=16 : zoomBook-= 8;
        bookmarksList->setIconSize(QSize(zoomBook,zoomBook));
        zoomLevel = zoomBook;
    } else {
        if(stackWidget->currentIndex() == 0) {
            if(iconAct->isChecked()) {
                (zoom == 16) ? zoom=16 : zoom-= 8;
                zoomLevel = zoom;
            } else {
                (zoomList == 16) ? zoomList=16 : zoomList-= 8;
                zoomLevel = zoomList;
            }
            toggleIcons();
        } else {
            (zoomDetail == 16) ? zoomDetail=16 : zoomDetail-= 8;
            detailTree->setIconSize(QSize(zoomDetail,zoomDetail));
            zoomLevel = zoomDetail;
        }
    }

    status->showMessage(QString(tr("Zoom: %1")).arg(zoomLevel));
}

//---------------------------------------------------------------------------
void MainWindow::focusAction()
{
    QAction *which = qobject_cast<QAction*>(sender());
    if(which)
    {
        if(which->text().contains("address")) pathEdit->setFocus(Qt::TabFocusReason);
        else if(which->text().contains("tree")) tree->setFocus(Qt::TabFocusReason);
        else if(which->text().contains("bookmarks")) bookmarksList->setFocus(Qt::TabFocusReason);
        else if(currentView == 2) detailTree->setFocus(Qt::TabFocusReason);
        else list->setFocus(Qt::TabFocusReason);
    }
    else
    {
        QApplication::clipboard()->blockSignals(0);
        pathEdit->setCompleter(customComplete);
    }
}
//---------------------------------------------------------------------------

void MainWindow::addressChanged(int old, int now)
{
    Q_UNUSED(old)

    if(!pathEdit->hasFocus()) return;
    QString temp = pathEdit->currentText();

    if(temp.contains("/."))
        if(!hiddenAct->isChecked())
        {
            hiddenAct->setChecked(1);
            toggleHidden();
        }

    if(temp.right(1) == "/")
    {
        modelList->index(temp);     //make sure model has walked this folder
        modelTree->invalidate();
    }

    if(temp.length() == now) return;
    int pos = temp.indexOf("/",now);

    pathEdit->lineEdit()->blockSignals(1);

    if(QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(temp.left(pos))));
    }
    else
    if(QApplication::mouseButtons() == Qt::MidButton)
    {
        QApplication::clipboard()->blockSignals(1);
        QApplication::clipboard()->clear(QClipboard::Selection);        //don't paste stuff

        pathEdit->setCompleter(0);
        tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(temp.left(pos))));

        QTimer::singleShot(500,this,SLOT(focusAction()));
    }
    else
    if(!pathEdit->lineEdit()->hasSelectedText())
    {
        pathEdit->completer()->setCompletionPrefix(temp.left(pos) + "/");
        pathEdit->completer()->complete();
    }

    pathEdit->lineEdit()->blockSignals(0);
}
//---------------------------------------------------------------------------
