#include "mainwindow.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QClipboard>
#include <QItemSelection>
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif
#include <fcntl.h>
#include <cstdio>

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
    QStringList args;
    if (filePath.endsWith(".desktop")) {
        DesktopFile df(filePath);
        args = QProcess::splitCommand(df.getExec());
        // drop field codes, there are no files to pass
        static const QStringList codes = {"%f", "%F", "%u", "%U",
                                          "%i", "%c", "%k"};
        for (const QString &code : codes) { args.removeAll(code); }
        if (args.isEmpty()) { return; }
        filePath = args.takeFirst();
    }
    qDebug() << "RUN" << filePath << args;
    QProcess::startDetached(filePath, args);
#endif
  } else {
    mimeUtils->openInApp(QFileInfo(filePath), term);
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

  // The history is the list of the path edit, most recent (current) first
  if (pathEdit->count() < 2) return;
  const QString current = pathEdit->itemText(0);

  // Previous folder that still exists (folders can have been removed)
  int target = -1;
  for (int i = 1; i < pathEdit->count(); ++i) {
    const QString path = pathEdit->itemText(i);
    if (path != current && QFileInfo(path).isDir()) {
      target = i;
      break;
    }
  }
  if (target == -1) return; // nowhere to go back to

  // Going back to a parent folder: select the folder we come from
  if (current.contains(pathEdit->itemText(target))) {
    backIndex = modelList->index(current);
  }

  // Remove the newer entries from the history
  for (int i = 0; i < target; ++i) {
    pathEdit->removeItem(0);
    if (tabs->count()) tabs->remHistory();
  }
  pathEdit->setCurrentIndex(0);
  forwardHistory.prepend(current);

  // Sets new dir index
  QModelIndex i = modelList->index(pathEdit->itemText(0));
  navigatingHistory = true;
  tree->setCurrentIndex(modelTree->mapFromSource(i));
  navigatingHistory = false;
}
//---------------------------------------------------------------------------

/**
 * @brief Goes forward again, to the folder left with "Back"
 */
void MainWindow::goForwardDir() {
  while (!forwardHistory.isEmpty()) {
    const QString path = forwardHistory.takeFirst();
    if (!QFileInfo(path).isDir()) { continue; } // removed meanwhile

    // the folder is added to the history by treeSelectionChanged()
    QModelIndex i = modelList->index(path);
    navigatingHistory = true;
    tree->setCurrentIndex(modelTree->mapFromSource(i));
    navigatingHistory = false;
    return;
  }
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
  QPersistentModelIndex created = modelList->insertFolder(i);
  if (!created.isValid()) { return; }
  newDir = modelView->mapFromSource(created);
  listSelectionModel->setCurrentIndex(newDir,
                                      QItemSelectionModel::ClearAndSelect);

  // Ask the name of the new directory
  renameDialog(created);
  selectListItem(created);
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
  QPersistentModelIndex created = modelList->insertFile(i);
  if (!created.isValid()) { return; }
  fileIndex = modelView->mapFromSource(created);
  listSelectionModel->setCurrentIndex(fileIndex,
                                      QItemSelectionModel::ClearAndSelect);

  // Ask the name of the new file
  renameDialog(created);
  selectListItem(created);
}
//---------------------------------------------------------------------------

/**
 * @brief Deletes file
 */
void MainWindow::deleteFile() {

  // The bookmarks have their own actions; do not touch the selected files
  if (focusWidget() == bookmarksList) { return; }

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

  bool ok = true;
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

  // Keep the paths, not the indexes: the confirmation dialogs below run the
  // event loop, where a change in the folder can destroy model items
  QStringList paths;
  foreach (QModelIndex item, selList) {
    const QString path = modelList->filePath(item);
    if (!path.isEmpty() && !paths.contains(path)) { paths.append(path); }
  }

  // Delete selected file(s)
  for (int i = 0; i < paths.count(); ++i) {
    QFileInfo file(paths.at(i));
    if (!file.exists() && !file.isSymLink()) { continue; } // already gone
    // removing an entry depends on the permission of the folder holding it
    if (!QFileInfo(file.absolutePath()).isWritable()) {
        ok = false;
        continue;
    }
    {
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
            ok &= QFile::remove(file.filePath());
        } else {
            const QModelIndex item = modelList->index(file.filePath());
            if (item.isValid()) { ok &= modelList->remove(item); }
            else { ok = false; }
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
    // The bookmarks have their own actions; do not touch the selected files
    if (focusWidget() == bookmarksList) { return; }

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
        // moving an entry depends on the permission of the folder holding it
        if (!QFileInfo(file.absolutePath()).isWritable()) {
            ok = false;
            continue;
        }
        QString trashPath = QString("%1/%2").arg(trashDir).arg(file.absoluteFilePath().split("/").takeLast());
        while (QFile::exists(trashPath) || QFileInfo(trashPath).isSymLink()) {
            trashPath = QString("%1/%2.%3").arg(trashDir).arg(file.absoluteFilePath().split("/").takeLast()).arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmssz"));
        }
        // rename(2) moves folders, files, special files and symlinks (also
        // broken ones, which QFile::rename() refuses) as they are
        if (::rename(QFile::encodeName(file.absoluteFilePath()).constData(),
                     QFile::encodeName(trashPath).constData()) != 0) {
            // rename(2) cannot cross file systems; regular files are then
            // copied and removed by QFile::rename()
            if (file.isSymLink() || !file.isFile()
                || !QFile::rename(file.absoluteFilePath(), trashPath)) {
                ok = false;
            }
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

  // The bookmarks have their own actions; do not touch the selected files
  if (focusWidget() == bookmarksList) { return; }

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
 * @brief Copies the full path of the selected items (or of the current
 * folder when nothing is selected) to the clipboard, one per line
 */
void MainWindow::copyPath() {
  QStringList paths;
  if (focusWidget() == list || focusWidget() == detailTree) {
    QModelIndexList selList = listSelectionModel->selectedRows(0);
    if (selList.isEmpty()) { selList = listSelectionModel->selectedIndexes(); }
    foreach (QModelIndex item, selList) {
      const QString path = modelList->filePath(modelView->mapToSource(item));
      if (!paths.contains(path)) { paths.append(path); }
    }
  }
  if (paths.isEmpty()) { paths.append(pathEdit->itemText(0)); }
  QApplication::clipboard()->setText(paths.join("\n"));
  status->showMessage(tr("Path copied to the clipboard"), 3000);
}
//---------------------------------------------------------------------------

/**
 * @brief Selects all items of the current folder
 */
void MainWindow::selectAllItems() {
  const QModelIndex root = (currentView == 2) ? detailTree->rootIndex() : list->rootIndex();
  const int rows = modelView->rowCount(root);
  if (rows < 1) { return; }
  QItemSelection all(modelView->index(0, 0, root), modelView->index(rows - 1, 0, root));
  listSelectionModel->select(all, QItemSelectionModel::Select);
}
//---------------------------------------------------------------------------

/**
 * @brief Selects the items that are not selected and deselects the others
 */
void MainWindow::invertSelection() {
  const QModelIndex root = (currentView == 2) ? detailTree->rootIndex() : list->rootIndex();
  const int rows = modelView->rowCount(root);
  if (rows < 1) { return; }
  QItemSelection all(modelView->index(0, 0, root), modelView->index(rows - 1, 0, root));
  listSelectionModel->select(all, QItemSelectionModel::Toggle);
}
//---------------------------------------------------------------------------

/**
 * @brief Shows the filter bar and puts the focus in it
 */
void MainWindow::showFilterBar() {
  filterBar->show();
  filterEdit->setFocus();
  filterEdit->selectAll();
}
//---------------------------------------------------------------------------

/**
 * @brief Clears the filter, hides the filter bar and gives the focus back to
 * the file view
 */
void MainWindow::closeFilterBar() {
  filterEdit->clear();
  filterBar->hide();
  if (currentView == 2) { detailTree->setFocus(); }
  else { list->setFocus(); }
}
//---------------------------------------------------------------------------

/**
 * @brief Applies the text of the filter bar to the file views
 */
void MainWindow::applyFilter() {
  const QString text = filterEdit->text();
  modelView->setNameFilter(text, filterDirsCheck->isChecked());
  if (text.isEmpty()) {
    status->clearMessage();
    return;
  }

  // visible items of the current folder, without ".."
  const QModelIndex root = (currentView == 2) ? detailTree->rootIndex() : list->rootIndex();
  int shown = modelView->rowCount(root);
  for (int i = 0; i < modelView->rowCount(root); ++i) {
    if (modelList->isDotDot(modelView->mapToSource(modelView->index(i, 0, root)))) { --shown; }
  }
  status->showMessage(tr("Filter: %1 items shown").arg(shown));
}
//---------------------------------------------------------------------------

/**
 * @brief Renames file
 */
void MainWindow::renameFile() {
  if (focusWidget() == bookmarksList) {
    // the label of a bookmark, not a file: edited in place
    bookmarksList->edit(bookmarksList->currentIndex());
  } else if (focusWidget() == tree) {
    QPersistentModelIndex source = modelTree->mapToSource(treeSelectionModel->currentIndex());
    if (renameDialog(source)) {
      // the renamed folder is the one being shown: load it under its new path
      treeSelectionChanged(treeSelectionModel->currentIndex(), QModelIndex());
    }
  } else if (focusWidget() == list || focusWidget() == detailTree) {
    // rename the current item only if it is part of the selection
    if (!listSelectionModel->hasSelection()) { return; }
    QModelIndex current = listSelectionModel->currentIndex();
    if (!listSelectionModel->isSelected(current)) {
      current = listSelectionModel->selectedIndexes().value(0);
    }
    QPersistentModelIndex source = modelView->mapToSource(current.sibling(current.row(), 0));
    if (renameDialog(source)) { selectListItem(source); }
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Selects and shows an item in the current list/detail view
 * @param sourceIndex index of the item in the file model
 */
void MainWindow::selectListItem(const QModelIndex &sourceIndex) {
  if (!sourceIndex.isValid()) { return; }
  const QModelIndex index = modelView->mapFromSource(sourceIndex);
  if (!index.isValid()) { return; }
  listSelectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
  if (stackWidget->currentIndex() == 0) { list->scrollTo(index); }
  else { detailTree->scrollTo(index); }
}
//---------------------------------------------------------------------------

/**
 * @brief Asks for the new name of a file or folder in a modal dialog, showing
 * the current name, and renames it
 * @param sourceIndex index of the item in the file model
 * @return true if the item was renamed
 */
bool MainWindow::renameDialog(const QModelIndex &sourceIndex) {
  // the item can go away while the dialog is open
  QPersistentModelIndex index(sourceIndex);
  if (!index.isValid()) { return false; }

  const QFileInfo info = modelList->fileInfo(index);
  const QString oldName = modelList->fileName(index);
  if (oldName.isEmpty() || oldName == "/") { return false; }

  QDialog dialog(this);
  dialog.setWindowTitle(tr("Rename"));
  dialog.setModal(true);
  dialog.setMinimumWidth(420);

  QLabel *currentLabel = new QLabel(tr("Current name: <b>%1</b>").arg(oldName.toHtmlEscaped()), &dialog);
  currentLabel->setTextFormat(Qt::RichText);
  currentLabel->setWordWrap(true);
  QLabel *newLabel = new QLabel(tr("New name:"), &dialog);
  QLineEdit *edit = new QLineEdit(oldName, &dialog);
  QLabel *message = new QLabel(&dialog);
  message->setWordWrap(true);
  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                   Qt::Horizontal, &dialog);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->addWidget(currentLabel);
  layout->addSpacing(6);
  layout->addWidget(newLabel);
  layout->addWidget(edit);
  layout->addWidget(message);
  layout->addWidget(buttons);

  // Select the name without the extension, which is usually kept
  int selection = oldName.length();
  if (!info.isDir()) {
    const int dot = oldName.lastIndexOf('.');
    if (dot > 0) { selection = dot; }
  }
  edit->setSelection(0, selection);
  edit->setFocus();

  connect(edit, &QLineEdit::textChanged, message, &QLabel::clear);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
    const QString name = edit->text();
    if (name == oldName) { dialog.reject(); return; } // nothing to do
    QString error;
    if (name.isEmpty()) {
      error = tr("The name cannot be empty.");
    } else if (name.contains('/')) {
      error = tr("The name cannot contain \"/\".");
    } else if (name == "." || name == "..") {
      error = tr("\"%1\" is not a valid name.").arg(name);
    } else {
      const QString newPath = info.absolutePath() + "/" + name;
      if (QFileInfo::exists(newPath) || QFileInfo(newPath).isSymLink()) {
        error = tr("There is already an item named \"%1\" in this folder.").arg(name);
      }
    }
    if (!error.isEmpty()) {
      message->setText(error);
      edit->setFocus();
      return;
    }
    dialog.accept();
  });

  if (dialog.exec() != QDialog::Accepted) { return false; }

  const QString newName = edit->text();
  // setData() is protected in myModel, it is public in the base class
  QAbstractItemModel *model = modelList;
  if (!index.isValid() || !model->setData(index, newName, Qt::EditRole)) {
    QMessageBox::warning(this, tr("Rename failed"),
                         tr("Could not rename \"%1\" to \"%2\". You may not have the proper permissions.")
                         .arg(oldName, newName));
    return false;
  }
  return true;
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

  // ".." is only shown in the detail view
  modelView->setShowDotDot(false);

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
    modelView->setShowDotDot(true); // ".." only in the detail view
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
    modelView->setFilterRegularExpression("no");
    modelTree->setFilterRegularExpression("no");
  } else {
    modelView->setFilterRegularExpression("");
    modelTree->setFilterRegularExpression("");
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
  QString oldTheme = settings->value("fallbackTheme").toString();

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
    QString newTheme = settings->value("fallbackTheme").toString();
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
