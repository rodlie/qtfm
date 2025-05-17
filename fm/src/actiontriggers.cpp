#include "mainwindow.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QStatusBar>
#include <QToolBar>
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif
#include <fcntl.h>

/**
 * @brief Executes a file
 * @param index
 * @param run
 */
void MainWindow::executeFile(QModelIndex index, bool run) {

  // Index of file
  QModelIndex srcIndex = modelView->mapToSource(index);

   QString filePath = modelList->filePath(srcIndex);
   QString type = mimeUtils->getMimeType(filePath);
   if (type.endsWith("executable") || type.endsWith("appimage") || filePath.endsWith(".desktop")) { run = true; }

  // Run or open
  if (run) {
#ifdef Q_OS_MAC
    QProcess::startDetached(QString("open \"%1\"").arg(filePath));
#else
    if (filePath.endsWith(".desktop")) {
        DesktopFile df(filePath);
        if (!df.getExec().isEmpty()) {
            filePath = df.getExec();
            if (filePath.toLower().contains("%f")) {
              filePath.replace("%f", "", Qt::CaseInsensitive);
            } else if (filePath.toLower().contains("%u")) {
              filePath.replace("%u", "", Qt::CaseInsensitive);
            }
            filePath = filePath.trimmed();
        } else { return; }
    }
    if (filePath.contains(" ")) {
        filePath.prepend("\"");
        filePath.append("\"");
    }
    qDebug() << "RUN" << filePath;
    QProcess::startDetached(filePath, QStringList());
#endif
  } else {
    mimeUtils->openInApp(filePath, ""/*term*/);
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Runs a file
 */
void MainWindow::runFile() {
    qDebug() << "runFile";
  executeFile(listSelectionModel->currentIndex(), 1);
}
//---------------------------------------------------------------------------

/**
 * @brief Opens folder
 */
void MainWindow::openFolderAction() {
  QModelIndex i = listSelectionModel->currentIndex();
  tree->setCurrentIndex(modelTree->mapFromSource(i));
}
//---------------------------------------------------------------------------

/**
 * @brief Opens file or files
 */
void MainWindow::openFile()
{
    qDebug() << "openFile(s)";

    // get selection
    QModelIndexList items;
    if (listSelectionModel->selectedRows(0).count()) {
        items = listSelectionModel->selectedRows(0);
    } else {
        items = listSelectionModel->selectedIndexes();
    }

    // get files and mimes
    QMap<QString,QString> files;
    QMap<QString,QString> mimes;
    foreach (QModelIndex index, items) {
        QModelIndex srcIndex = modelView->mapToSource(index);
        QString filePath = modelList->filePath(srcIndex);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isDir()) { continue; }
        QString mime = mimeUtils->getMimeType(filePath);
        if (mime.isEmpty()) { continue; }
        files[filePath] = mime;
        mimes[mime] = "";
    }
    qDebug() << "selected files" << items.size() << files << mimes;

    // get apps for mimes
    QMapIterator<QString, QString> i_mimes(mimes);
    while (i_mimes.hasNext()) {
        i_mimes.next();
        QString app = mimeUtils->getAppForMimeType(i_mimes.key());
        if (app.isEmpty()) { continue; }
        mimes[i_mimes.key()] = app;
    }
    qDebug() << "selected files apps" << mimes;

    // match apps and files
    QMap<QString,QStringList> launch;
    QMapIterator<QString, QString> i_apps(mimes);
    while (i_apps.hasNext()) {
        i_apps.next();
        QString app = i_apps.value();
        QString mime = i_apps.key();
        if (app.isEmpty()) { continue; }
        QMapIterator<QString, QString> i_files(files);
        while (i_files.hasNext()) {
            i_files.next();
            if (mime == i_files.value()) { launch[app] << i_files.key(); }
        }
    }
    qDebug() << "launch" << launch;

    // launch
    QMapIterator<QString, QStringList> i_launch(launch);
    while (i_launch.hasNext()) {
        i_launch.next();
        QString desktop = Common::findApplication(qApp->applicationFilePath(), i_launch.key());
        if (desktop.isEmpty()) { continue; }
        DesktopFile df = DesktopFile(desktop);
        if (df.getExec().isEmpty()) { continue; }
        QStringList fileList = i_launch.value();
        if (df.getExec().contains("%F") || df.getExec().contains("%U")) { // app supports multiple files
            mimeUtils->openFilesInApp(df.getExec(), fileList, df.isTerminal()?term:"");
        } else { // launch new instance for each file
            for (int i=0;i<i_launch.value().size();++i) {
                QFileInfo fileInfo(fileList.at(i));
                mimeUtils->openInApp(df.getExec(), fileInfo, df.isTerminal()?term:"");
            }
        }
    }
}

//---------------------------------------------------------------------------

/**
 * @brief Goes up in directory tree
 */
void MainWindow::goUpDir() {
  tree->setCurrentIndex(tree->currentIndex().parent());
}
//---------------------------------------------------------------------------

/**
 * @brief Goes back in directory tree
 */
void MainWindow::goBackDir() {

  // If there is only one item in path edit, we cannot go back
  if (pathEdit->count() == 1) return;

  // Retrieve current index
  QString current = pathEdit->currentText();
  if (current.contains(pathEdit->itemText(1))) {
    backIndex = modelList->index(current);
  }

  // Remove history
  do {
    pathEdit->removeItem(0);
    if (tabs->count()) tabs->remHistory();
  } while (!QFileInfo(pathEdit->itemText(0)).exists()
           || pathEdit->itemText(0) == current);

  // Sets new dir index
  QModelIndex i = modelList->index(pathEdit->itemText(0));
  tree->setCurrentIndex(modelTree->mapFromSource(i));
}
//---------------------------------------------------------------------------

/**
 * @brief Goes to home directory
 */
void MainWindow::goHomeDir() {
  QModelIndex i = modelTree->mapFromSource(modelList->index(QDir::homePath()));
  tree->setCurrentIndex(i);
}
//---------------------------------------------------------------------------

/**
 * @brief Starts terminal
 */
void MainWindow::terminalRun() {

  // If terminal was not specified, asks user for terminal command
  if (term.isEmpty()) {
    QString title = tr("Setting");
    QString label = tr("Set default terminal:");
    QString def = "xterm";
#ifdef Q_OS_MAC
    def = "/Applications/Utilities/Terminal.app/Contents/MacOS/Terminal";
#endif
    term = QInputDialog::getText(this, title, label, QLineEdit::Normal, def);
    settings->setValue("term", term);
  }

  // Starts terminal
  QStringList args(term.split(" "));
  QString name = args.at(0);
  args.removeAt(0);
  QProcess::startDetached(name, args, pathEdit->itemText(0));
}
//---------------------------------------------------------------------------

/**
 * @brief Creates a new directory
 */
void MainWindow::newDir() {

  // Check whether current directory is writeable
  QModelIndex newDir;
  if (!QFileInfo(pathEdit->itemText(0)).isWritable()) {
    status->showMessage(tr("The current directory is not writable, unable to create new folder."));
    return;
  }

  // Create new directory
  QModelIndex i = modelList->index(pathEdit->itemText(0));
  newDir = modelView->mapFromSource(modelList->insertFolder(i));
  listSelectionModel->setCurrentIndex(newDir,
                                      QItemSelectionModel::ClearAndSelect);

  // Editation of name of new directory
  if (stackWidget->currentIndex() == 0) list->edit(newDir);
  else detailTree->edit(newDir);
}
//---------------------------------------------------------------------------

/**
 * @brief Creates a new file
 */
void MainWindow::newFile() {

  // Check whether current directory is writeable
  QModelIndex fileIndex;
  if (!QFileInfo(pathEdit->itemText(0)).isWritable()) {
    status->showMessage(tr("The current directory is not writable, unable to create new file."));
    return;
  }

  // Create new file
  QModelIndex i = modelList->index(pathEdit->itemText(0));
  fileIndex = modelView->mapFromSource(modelList->insertFile(i));
  listSelectionModel->setCurrentIndex(fileIndex,
                                      QItemSelectionModel::ClearAndSelect);

   // Editation of name of new file
  if (stackWidget->currentIndex() == 0) list->edit(fileIndex);
  else detailTree->edit(fileIndex);
}
//---------------------------------------------------------------------------

/**
 * @brief Deletes file
 */
void MainWindow::deleteFile() {

  // Temporary selection info
  QModelIndexList selList;
  bool yesToAll = false;

  // Retrieves selection
  if (focusWidget() == tree) {
    selList << modelList->index(pathEdit->itemText(0));
  } else {
    QModelIndexList proxyList;
    if (listSelectionModel->selectedRows(0).count()) {
      proxyList = listSelectionModel->selectedRows(0);
    } else {
      proxyList = listSelectionModel->selectedIndexes();
    }
    foreach (QModelIndex proxyItem, proxyList) {
      selList.append(modelView->mapToSource(proxyItem));
    }
  }

  bool ok = false;
  bool confirm;

  // Display confirmation message box
  if (settings->value("confirmDelete").isNull()) {
    QString title = tr("Delete confirmation");
    QString msg = tr("Confirm all delete operations?");
    QMessageBox::StandardButtons btns = QMessageBox::Yes | QMessageBox::No;
    if (QMessageBox::question(this, title, msg, btns) == QMessageBox::Yes) {
      confirm = 1;
    } else {
      confirm = 0;
    }
    settings->setValue("confirmDelete",confirm);
  } else {
    confirm = settings->value("confirmDelete").toBool();
  }

  // Delete selected file(s)
  for (int i = 0; i < selList.count(); ++i) {
    QFileInfo file(modelList->filePath(selList.at(i)));
    if (file.isWritable() || file.isSymLink()) {
        if (yesToAll == false) {
          if (confirm) {
            QString title = tr("Careful");
            QString msg = tr("Are you sure you want to delete <p><b>\"") +
                          file.filePath() + "</b>?";
            int ret = QMessageBox::information(this, title, msg,
                QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
            if (ret == QMessageBox::YesToAll) yesToAll = true;
            if (ret == QMessageBox::No) return;
          }
        }
        if (file.isSymLink()) {
            ok = QFile::remove(file.filePath());
        } else {
            ok = modelList->remove(selList.at(i));
        }
    }
  }

  // Display error message if deletion failed
  if(!ok) {
    QString title = tr("Failed");
    QString msg = tr("Some files where not deleted. You may not have the proper permissions or maybe the file system is read-only.");
    QMessageBox::warning(this, title, msg);
  }

  return;
}

void MainWindow::trashFile()
{
    // Temporary selection files
    QModelIndexList selList;
    QStringList fileList;

    // Selection
    if (focusWidget() == tree) {
      selList << modelView->mapFromSource(modelList->index(pathEdit->itemText(0)));
    } else if (listSelectionModel->selectedRows(0).count()) {
      selList = listSelectionModel->selectedRows(0);
    } else {
      selList = listSelectionModel->selectedIndexes();
    }

    // Retrieve selected indices
    foreach (QModelIndex item, selList) {
      fileList.append(modelList->filePath(modelView->mapToSource(item)));
    }

    bool ok = true;
    for (int i=0;i<fileList.size();++i) {
        QFileInfo file(fileList.at(i));
        if (!file.isWritable()) {
            ok = false;
            continue;
        }
        QString trashPath = QString("%1/%2").arg(trashDir).arg(file.absoluteFilePath().split("/").takeLast());
        while (QFile::exists(trashPath)) {
            trashPath = QString("%1/%2.%3").arg(trashDir).arg(file.absoluteFilePath().split("/").takeLast()).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmssz"));
        }
        if (file.isDir()) {
            QDir origDir(file.absoluteFilePath());
            bool movedDir = origDir.rename(file.absoluteFilePath(), trashPath);
            if (!movedDir) { ok = false; }
        } else if (file.isFile()) {
            QFile origFile(file.absoluteFilePath());
            bool movedFile = origFile.rename(file.absoluteFilePath(), trashPath);
            if (!movedFile) { ok = false; }
        }
    }
    if(!ok) {
      QString title = tr("Failed");
      QString msg = tr("Some files where not moved. You may not have the proper permissions or maybe the file system is read-only.");
      QMessageBox::warning(this, title, msg);
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Cuts file
 */
void MainWindow::cutFile() {

  // Temporary selection files
  QModelIndexList selList;
  QStringList fileList;

  // Selection
  if (focusWidget() == tree) {
    selList << modelView->mapFromSource(modelList->index(pathEdit->itemText(0)));
  } else if (listSelectionModel->selectedRows(0).count()) {
    selList = listSelectionModel->selectedRows(0);
  } else {
    selList = listSelectionModel->selectedIndexes();
  }

  // Retrieve selected indices
  foreach (QModelIndex item, selList) {
    fileList.append(modelList->filePath(modelView->mapToSource(item)));
  }

  clearCutItems();
  modelList->addCutItems(fileList);

  // Save a temp file to allow pasting in a different instance
  const QString clipboardFile = Common::getTempClipboardFile();
  if (!clipboardFile.isEmpty()) {
      QFile tempFile(clipboardFile);
      tempFile.open(QIODevice::WriteOnly);
      QDataStream out(&tempFile);
      out << fileList;
      tempFile.close();
  }

  QApplication::clipboard()->setMimeData(modelView->mimeData(selList));

  modelTree->invalidate();
  listSelectionModel->clear();
}
//---------------------------------------------------------------------------

/**
 * @brief Copies a file
 */
void MainWindow::copyFile() {

  // Selection
  QModelIndexList selList;
  if (listSelectionModel->selectedRows(0).count()) {
    selList = listSelectionModel->selectedRows(0);
  } else {
    selList = listSelectionModel->selectedIndexes();
  }

  if (selList.count() == 0) {
    if (focusWidget() == tree) {
      QModelIndex i = modelList->index(pathEdit->itemText(0));
      selList << modelView->mapFromSource(i);
    } else {
      return;
    }
  }

  clearCutItems();

  QStringList text;
  foreach (QModelIndex item,selList) {
    text.append(modelList->filePath(modelView->mapToSource(item)));
  }

  QApplication::clipboard()->setText(text.join("\n"), QClipboard::Selection);
  QApplication::clipboard()->setMimeData(modelView->mimeData(selList));

  cutAct->setData(0);
}
//---------------------------------------------------------------------------

/**
 * @brief Renames file
 */
void MainWindow::renameFile() {
  if (focusWidget() == tree) {
    tree->edit(treeSelectionModel->currentIndex());
  } else if(focusWidget() == bookmarksList) {
    bookmarksList->edit(bookmarksList->currentIndex());
  } else if(focusWidget() == list) {
    list->edit(listSelectionModel->currentIndex());
  } else if(focusWidget() == detailTree) {
    detailTree->edit(listSelectionModel->currentIndex());
  }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

/**
 * @brief Creates symbolic links to files
 * @param files
 * @param newPath
 * @return true if link creation was successful
 */
bool MainWindow::linkFiles(const QList<QUrl> &files, const QString &newPath) {

  // Quit if folder not writable
  if (!QFileInfo(newPath).isWritable()
      || newPath == QDir(files.at(0).toLocalFile()).path())
  {
      QMessageBox::warning(this, tr("Folder not writable"), tr("The destination folder (%1) is not writable").arg(newPath));
      return false;
  }

  // TODO: even if symlinks are small we have to make sure that we have space
  // available for links

  // Main loop
  for (int i = 0; i < files.count(); ++i) {

    // Choose destination file name and url
    QFile file(files.at(i).toLocalFile());
    QFileInfo temp(file);
    QString destName = temp.fileName();
    QString destUrl = newPath + QDir::separator() + destName;

    // Only do 'Link(x) of' if same folder
    if (temp.path() == newPath) {
      int num = 1;
      while (QFile(destUrl).exists()) {
        destName = QString("Link (%1) of %2").arg(num).arg(temp.fileName());
        destUrl = newPath + QDir::separator() + destName;
        num++;
      }
    }

    // If file does not exists then create link
    QFileInfo dName(destUrl);
    if (!dName.exists()) {
      file.link(destUrl);
    }
  }
  return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Locks/Unlocks editation of layout
 */
void MainWindow::toggleLockLayout() {

  if (lockLayoutAct->isChecked()) {
    QFrame *newTitle = new QFrame();
    newTitle->setFrameShape(QFrame::StyledPanel);
    newTitle->setMinimumSize(0,1);
    dockTree->setTitleBarWidget(newTitle);

    newTitle = new QFrame();
    newTitle->setFrameShape(QFrame::StyledPanel);
    newTitle->setMinimumSize(0,1);
    dockBookmarks->setTitleBarWidget(newTitle);

    newTitle = new QFrame();
    newTitle->setFrameShape(QFrame::StyledPanel);
    newTitle->setMinimumSize(0,1);

    menuToolBar->setMovable(0);
    //editToolBar->setMovable(0);
    //viewToolBar->setMovable(0);
    navToolBar->setMovable(0);
    addressToolBar->setMovable(0);
    lockLayoutAct->setText(tr("Unlock layout"));
  } else {
    dockTree->setTitleBarWidget(nullptr);
    dockBookmarks->setTitleBarWidget(nullptr);

    menuToolBar->setMovable(1);
    //editToolBar->setMovable(1);
    //viewToolBar->setMovable(1);
    navToolBar->setMovable(1);
    addressToolBar->setMovable(1);

    lockLayoutAct->setText(tr("Lock layout"));
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Toggles icon view (thumbs and sizes)
 */
void MainWindow::toggleIcons() {

  // Set root index
  if (list->rootIndex() != modelList->index(pathEdit->currentText())) {
    QModelIndex i = modelList->index(pathEdit->currentText());
    list->setRootIndex(modelView->mapFromSource(i));
  }

  if (iconAct->isChecked()) {
    currentView = 1;
    list->setViewMode(QListView::IconMode);
    list->setItemDelegate(ivdelegate);
    /*int padding = 10;
    if (zoom<48) { padding = 20; }
    if (zoom<32) { padding = 40; }
    if (zoom<24) { padding = 50; }
    list->setGridSize(QSize(zoom*2+padding, zoom*2+padding));*/
    list->setGridSize(QSize(zoom, zoom));
    list->setIconSize(QSize(zoom, zoom));
    list->setFlow(QListView::LeftToRight);

    modelList->setMode(thumbsAct->isChecked());
    stackWidget->setCurrentIndex(0);

    detailAct->setChecked(0);
    detailTree->setMouseTracking(false);
    list->setMouseTracking(true);

    if (tabs->count()) tabs->setType(1);
    updateGrid();
  } else {
    currentView = 0;
    list->setViewMode(QListView::ListMode);
    list->setItemDelegate(ildelegate);
    list->setGridSize(QSize());
    list->setIconSize(QSize(zoomList, zoomList));
    list->setFlow(QListView::TopToBottom);

    modelList->setMode(thumbsAct->isChecked());
    list->setMouseTracking(false);

    if (tabs->count()) tabs->setType(0);
  }

  list->setDragDropMode(QAbstractItemView::DragDrop);
  list->setDefaultDropAction(Qt::MoveAction);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets sort column
 * @param columnAct
 */
void MainWindow::setSortColumn(QAction *columnAct) {

  // Set root index
  if (list->rootIndex() != modelList->index(pathEdit->currentText())) {
    QModelIndex i = modelList->index(pathEdit->currentText());
    list->setRootIndex(modelView->mapFromSource(i));
  }

  columnAct->setChecked(true);

  if (columnAct == sortNameAct) {
    currentSortColumn =  0;
  } else if (columnAct == sortDateAct) {
    currentSortColumn =  3;
  } else if (columnAct == sortSizeAct) {
    currentSortColumn = 1;
  }
  settings->setValue("sortBy", currentSortColumn);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets sort column
 * @param action
 */
void MainWindow::toggleSortBy(QAction *action) {
  setSortColumn(action);
  modelView->sort(currentSortColumn, currentSortOrder);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets sort order
 * @param order
 */
void MainWindow::setSortOrder(Qt::SortOrder order) {

  // Set root index
  if (list->rootIndex() != modelList->index(pathEdit->currentText())) {
    QModelIndex i = modelList->index(pathEdit->currentText());
    list->setRootIndex(modelView->mapFromSource(i));
  }

  // Change sort order
  currentSortOrder = order;
  sortAscAct->setChecked(!((bool) currentSortOrder));
  settings->setValue("sortOrder", currentSortOrder);
}
//---------------------------------------------------------------------------

/**
 * @brief Changes sort order
 */
void MainWindow::toggleSortOrder() {
  setSortOrder(currentSortOrder == Qt::AscendingOrder ? Qt::DescendingOrder
                                                      : Qt::AscendingOrder);
  modelView->sort(currentSortColumn, currentSortOrder);
}
//---------------------------------------------------------------------------

/**
 * @brief Switches from thumbs to details and vice versa
 */
void MainWindow::toggleThumbs() {
  if (currentView != 2) toggleIcons();
  else toggleDetails();
}
//---------------------------------------------------------------------------

/**
 * @brief Toggles details
 */
void MainWindow::toggleDetails() {
  if (detailAct->isChecked() == false) {
    toggleIcons();
    stackWidget->setCurrentIndex(0);
    detailTree->setMouseTracking(false);
  } else {
    currentView = 2;
    QModelIndex i = modelList->index(pathEdit->currentText());
    if (detailTree->rootIndex() != i) {
      detailTree->setRootIndex(modelView->mapFromSource(i));
    }
    detailTree->setMouseTracking(true);
    stackWidget->setCurrentIndex(1);
    modelList->setMode(thumbsAct->isChecked());
    iconAct->setChecked(0);
    if (tabs->count()) {
      tabs->setType(2);
    }
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Hides/Shows hidden files
 */
void MainWindow::toggleHidden() {

  if (hiddenAct->isChecked() == false) {
    if (curIndex.isHidden()) {
      listSelectionModel->clear();
    }
    modelView->setFilterRegExp("no");
    modelTree->setFilterRegExp("no");
  } else {
    modelView->setFilterRegExp("");
    modelTree->setFilterRegExp("");
  }

  modelView->invalidate();
  dirLoaded();
}
//---------------------------------------------------------------------------

/**
 * @brief Displays about box
 */
void MainWindow::showAboutBox()
{
    QMessageBox box;
    box.setWindowTitle(tr("About %1").arg(APP_NAME));
    box.setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/images/qtfm.png")));
    box.setIconPixmap(QPixmap::fromImage(QImage(":/images/qtfm.png")));
    box.setText(QString("<h1>%1 %2</h1>"
                        "<h3 style=\"font-weight:normal;\">Qt File Manager</h3>").arg(APP_NAME).arg(APP_VERSION));
    box.setInformativeText(QString("<p style=\"text-align:justify;font-size:small;\">"
                                   "This program is free software; you can redistribute it and/or modify"
                                   " it under the terms of the GNU General Public License as published by"
                                   " the Free Software Foundation; either version 2 of the License, or"
                                   " (at your option) any later version.</p>"
                                   "<p style=\"font-size:small;\">Copyright &copy;2010-2019 The QtFM Developers."
                                   "<br>All rights reserved.</p>"
                                   "<p style=\"font-weight:bold;\">"
                                   "<a href=\"https://qtfm.eu\">"
                                   "https://qtfm.eu</a></p>"));
    QString details;
    QFile authorsFile(":/AUTHORS");
    if (authorsFile.open(QIODevice::Text|QIODevice::ReadOnly)) {
        details.append(authorsFile.readAll());
        authorsFile.close();
    }
    if (!details.isEmpty()) { box.setDetailedText(details); }
    box.exec();
}
//---------------------------------------------------------------------------

/**
 * @brief Displays settings dialog
 */
void MainWindow::showEditDialog() {

  // Deletes current list of custom actions
  customActManager->freeActions();

  // Loads current icon theme
  QString oldTheme = settings->value("forceTheme").toString();

  // save settings
  writeSettings();

  // Creates settings dialog
  SettingsDialog *d = new SettingsDialog(actionList, settings, mimeUtils, this);
  if (d->exec()) {

    // Reload settings
    loadSettings(false /* don't reload window state/geo */,
                 false /* don't reload hidden state */,
                 false /* don't reload tabs state */,
                 false /* don't reload thumb state */);

    // If icon theme has changed, use new theme and clear cache
    QString newTheme = settings->value("forceTheme").toString();
    if (oldTheme.compare(newTheme) != 0) {
      modelList->clearIconCache();
      QIcon::setThemeName(newTheme);
    }
  }

  // Reads custom actions
  customActManager->readActions();
  delete d;
}
//---------------------------------------------------------------------------
