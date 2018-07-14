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
#include <QDockWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QApplication>
#include <QStatusBar>
#include <QMenu>
#ifndef NO_DBUS
#include <QDBusConnection>
#endif
#include <sys/vfs.h>
#include <fcntl.h>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#endif

#include "mainwindow.h"
#include "mymodel.h"
#include "progressdlg.h"
#include "fileutils.h"
#include "applicationdialog.h"

#include "common.h"

MainWindow::MainWindow()
{
    // libdisks
#ifndef NO_UDISKS
    disks = new Disks();
    connect(disks, SIGNAL(updatedDevices()), this, SLOT(populateMedia()));
    connect(disks, SIGNAL(mountpointChanged(QString,QString)), this, SLOT(handleMediaMountpointChanged(QString,QString)));
    connect(disks, SIGNAL(foundNewDevice(QString)), this, SLOT(handleMediaAdded(QString)));
    connect(disks, SIGNAL(removedDevice(QString)), this, SLOT(handleMediaRemoved(QString)));
    connect(disks, SIGNAL(mediaChanged(QString,bool)), this, SLOT(handleMediaChanged(QString,bool)));
#endif

    // dbus service
#ifndef NO_DBUS
    if (QDBusConnection::sessionBus().isConnected()) {
        if (QDBusConnection::sessionBus().registerService(FM_SERVICE)) {
            service = new qtfm();
            connect(service, SIGNAL(pathRequested(QString)), this, SLOT(handlePathRequested(QString)));
            if (!QDBusConnection::sessionBus().registerObject(FM_PATH, service, QDBusConnection::ExportAllSlots)) {
                qWarning() << QDBusConnection::sessionBus().lastError().message();
            }
        } else { qWarning() << QDBusConnection::sessionBus().lastError().message(); }
    }
#endif

    startPath = QDir::currentPath();
    QStringList args = QApplication::arguments();

    if(args.count() > 1) {
        startPath = args.at(1);
#if QT_VERSION >= 0x040800
        if(QUrl(startPath).isLocalFile()) {
            startPath = QUrl(args.at(1)).toLocalFile();
        }
#endif
    }

    settings = new QSettings(Common::configFile(), QSettings::IniFormat);
    if (settings->value("clearCache").toBool()) {
        qDebug() << "clear cache";
        Common::removeFileCache();
        Common::removeFolderCache();
        Common::removeThumbsCache();
        settings->setValue("clearCache", false);
    }
    // Dark theme
#if QT_VERSION >= 0x050000
    if (settings->value("darkTheme").toBool()) {
        qApp->setPalette(Common::darkTheme());
    }
#endif

    // set icon theme
    Common::setupIconTheme(qApp->applicationFilePath());

    // Create mime utils
    mimeUtils = new MimeUtils(this);
    QString tmp = "/.local/share/applications/mimeapps.list";
    QString name = settings->value("defMimeAppsFile", tmp).toString();
    mimeUtils->setDefaultsFileName(name);

    // Create filesystem model
    bool realMime = settings->value("realMimeTypes", true).toBool();
    modelList = new myModel(realMime, mimeUtils);
    connect(modelList, SIGNAL(reloadDir()), this, SLOT(dirLoaded()));

    dockTree = new QDockWidget(tr("Tree"),this,Qt::SubWindow);
    dockTree->setObjectName("treeDock");

    tree = new QTreeView(dockTree);
    dockTree->setWidget(tree);
    addDockWidget(Qt::LeftDockWidgetArea, dockTree);

    dockBookmarks = new QDockWidget(tr("Bookmarks"),this,Qt::SubWindow);
    dockBookmarks->setObjectName("bookmarksDock");
    dockBookmarks->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    bookmarksList = new QListView(dockBookmarks);
    bookmarksList->setMinimumHeight(24); // Docks get the minimum size from their content widget
    dockBookmarks->setWidget(bookmarksList);
    addDockWidget(Qt::LeftDockWidgetArea, dockBookmarks);

    QWidget *main = new QWidget;
    mainLayout = new QVBoxLayout(main);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    stackWidget = new QStackedWidget();
    QWidget *page = new QWidget();
    QHBoxLayout *hl1 = new QHBoxLayout(page);
    hl1->setSpacing(0);
    hl1->setContentsMargins(0,0,0,0);
    list = new QListView(page);
    hl1->addWidget(list);
    stackWidget->addWidget(page);

    QWidget *page2 = new QWidget();
    hl1 = new QHBoxLayout(page2);
    hl1->setSpacing(0);
    hl1->setContentsMargins(0,0,0,0);
    detailTree = new QTreeView(page2);
    hl1->addWidget(detailTree);
    stackWidget->addWidget(page2);

    tabs = new tabBar(modelList->folderIcons);

    mainLayout->addWidget(stackWidget);
    mainLayout->addWidget(tabs);

    setCentralWidget(main);

    modelTree = new mainTreeFilterProxyModel();
    modelTree->setSourceModel(modelList);
    modelTree->setSortCaseSensitivity(Qt::CaseInsensitive);

    tree->setHeaderHidden(true);
    tree->setUniformRowHeights(true);
    tree->setModel(modelTree);
    tree->hideColumn(1);
    tree->hideColumn(2);
    tree->hideColumn(3);
    tree->hideColumn(4);

    modelView = new viewsSortProxyModel();
    modelView->setSourceModel(modelList);
    modelView->setSortCaseSensitivity(Qt::CaseInsensitive);

    list->setWrapping(true);
    list->setWordWrap(true);
    list->setModel(modelView);
    ivdelegate = new IconViewDelegate();
    ildelegate = new IconListDelegate();
    list->setTextElideMode(Qt::ElideNone);
    listSelectionModel = list->selectionModel();

    detailTree->setRootIsDecorated(false);
    detailTree->setItemsExpandable(false);
    detailTree->setUniformRowHeights(true);
    detailTree->setModel(modelView);
    detailTree->setSelectionModel(listSelectionModel);

    pathEdit = new QComboBox();
    pathEdit->setEditable(true);
    pathEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    status = statusBar();
    status->setSizeGripEnabled(true);
    statusName = new QLabel();
    statusSize = new QLabel();
    statusDate = new QLabel();
    status->addPermanentWidget(statusName);
    status->addPermanentWidget(statusSize);
    status->addPermanentWidget(statusDate);

    treeSelectionModel = tree->selectionModel();
    connect(treeSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(treeSelectionChanged(QModelIndex,QModelIndex)));
    tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(startPath)));
    tree->scrollTo(tree->currentIndex());

    // applications dock
#ifndef NO_APPDOCK
    appDock = new ApplicationDock(this, Qt::SubWindow);
    appDock->setObjectName("appDock");
    addDockWidget(Qt::LeftDockWidgetArea, appDock);
#endif

    createActions();
    createToolBars();
    createMenus();

    setWindowIcon(QIcon::fromTheme("folder"));
    setWindowTitle(APP_NAME);

    // Create custom action manager
    customActManager = new CustomActionsManager(settings, actionList, this);

    // Create bookmarks model
    modelBookmarks = new bookmarkmodel(modelList->folderIcons);
    connect(modelBookmarks, SIGNAL(bookmarksChanged()), this, SLOT(handleBookmarksChanged()));

    // Load settings before showing window
    loadSettings();

    // show window
    show();

    trashDir = Common::trashDir();

    QTimer::singleShot(0, this, SLOT(lateStart()));
}
//---------------------------------------------------------------------------

/**
 * @brief Initialization
 */
void MainWindow::lateStart() {

  // Update status panel
  status->showMessage(getDriveInfo(curIndex.filePath()));

  // Configure bookmarks list
  bookmarksList->setDragDropMode(QAbstractItemView::DragDrop);
  bookmarksList->setDropIndicatorShown(false);
  bookmarksList->setDefaultDropAction(Qt::MoveAction);
  bookmarksList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Configure tree view
  tree->setDragDropMode(QAbstractItemView::DragDrop);
  tree->setDefaultDropAction(Qt::MoveAction);
  tree->setDropIndicatorShown(false);
  tree->setEditTriggers(QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::SelectedClicked);

  // Configure detail view
  detailTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
  detailTree->setDragDropMode(QAbstractItemView::DragDrop);
  detailTree->setDefaultDropAction(Qt::MoveAction);
  detailTree->setDropIndicatorShown(false);
  detailTree->setEditTriggers(QAbstractItemView::EditKeyPressed |
                              QAbstractItemView::SelectedClicked);

  // Configure list view
  list->setResizeMode(QListView::Adjust);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  list->setSelectionRectVisible(true);
  list->setFocus();
  list->setEditTriggers(QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::SelectedClicked);

  // Clipboard configuration
  progress = 0;
  clipboardChanged();

  // Completer configuration
  customComplete = new myCompleter;
  customComplete->setModel(modelTree);
  customComplete->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  customComplete->setMaxVisibleItems(10);
  pathEdit->setCompleter(customComplete);

  // Tabs configuration
  tabs->setDrawBase(0);
  tabs->setExpanding(0);

  // Connect mouse clicks in views
  if (settings->value("singleClick").toInt() == 1) {
    connect(list, SIGNAL(clicked(QModelIndex)),
            this, SLOT(listItemClicked(QModelIndex)));
    connect(detailTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(listItemClicked(QModelIndex)));
  }
  if (settings->value("singleClick").toInt() == 2) {
    connect(list, SIGNAL(clicked(QModelIndex))
            ,this, SLOT(listDoubleClicked(QModelIndex)));
    connect(detailTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(listDoubleClicked(QModelIndex)));
  }

  // Connect list view
  connect(list, SIGNAL(activated(QModelIndex)),
          this, SLOT(listDoubleClicked(QModelIndex)));

  // Connect custom action manager
  connect(customActManager, SIGNAL(actionMapped(QString)),
          SLOT(actionMapper(QString)));
  connect(customActManager, SIGNAL(actionsLoaded()), SLOT(readShortcuts()));
  connect(customActManager, SIGNAL(actionFinished()), SLOT(clearCutItems()));

  // Connect path edit
  connect(pathEdit, SIGNAL(activated(QString)),
          this, SLOT(pathEditChanged(QString)));
  connect(customComplete, SIGNAL(activated(QString)),
          this, SLOT(pathEditChanged(QString)));
  connect(pathEdit->lineEdit(), SIGNAL(cursorPositionChanged(int,int)),
          this, SLOT(addressChanged(int,int)));

  // Connect bookmarks
  connect(bookmarksList, SIGNAL(activated(QModelIndex)),
          this, SLOT(bookmarkClicked(QModelIndex)));
  connect(bookmarksList, SIGNAL(clicked(QModelIndex)),
          this, SLOT(bookmarkClicked(QModelIndex)));
  connect(bookmarksList, SIGNAL(pressed(QModelIndex)),
          this, SLOT(bookmarkPressed(QModelIndex)));

  // Connect selection
  connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
          this, SLOT(clipboardChanged()));
  connect(detailTree,SIGNAL(activated(QModelIndex)),
          this, SLOT(listDoubleClicked(QModelIndex)));
  connect(listSelectionModel,
          SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
          this, SLOT(listSelectionChanged(const QItemSelection,
                                          const QItemSelection)));

  // Connect copy progress
  connect(this, SIGNAL(copyProgressFinished(int,QStringList)),
          this, SLOT(progressFinished(int,QStringList)));

  // Connect bookmark model
  connect(modelBookmarks,
          SIGNAL(bookmarkPaste(const QMimeData *, QString, QStringList)), this,
          SLOT(pasteLauncher(const QMimeData *, QString, QStringList)));
  connect(modelBookmarks, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(readShortcuts()));
  connect(modelBookmarks, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(readShortcuts()));

  // Conect list model
  connect(modelList,
          SIGNAL(dragDropPaste(const QMimeData *, QString, Common::DragMode)),
          this,
          SLOT(dragLauncher(const QMimeData *, QString, Common::DragMode)));

  // Connect tabs
  connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
  connect(tabs, SIGNAL(dragDropTab(const QMimeData *, QString, QStringList)),
          this, SLOT(pasteLauncher(const QMimeData *, QString, QStringList)));
  connect(list, SIGNAL(pressed(QModelIndex)),
          this, SLOT(listItemPressed(QModelIndex)));
  connect(detailTree, SIGNAL(pressed(QModelIndex)),
          this, SLOT(listItemPressed(QModelIndex)));

  connect(modelList, SIGNAL(thumbUpdate(QModelIndex)),
          this, SLOT(thumbUpdate(QModelIndex)));

  qApp->setKeyboardInputInterval(1000);

  // Read custom actions
  QTimer::singleShot(100, customActManager, SLOT(readActions()));

  // Read defaults
  QTimer::singleShot(100, mimeUtils, SLOT(generateDefaults()));
}
//---------------------------------------------------------------------------

/**
 * @brief Loads application settings
 */
void MainWindow::loadSettings(bool wState, bool hState, bool tabState, bool thumbState) {

  // first run?
    bool isFirstRun = false;
    if (!settings->value("firstRun").isValid()) {
        isFirstRun = true;
        settings->setValue("firstRun", false);
    }

#if QT_VERSION >= 0x050000
  // fix style
  setStyleSheet("QToolBar { padding: 0;border:none; }"
                "QFrame { border:none; }"
                "QListView::item,QListView::text,QListView::icon"
                "{ border:0px;padding-top:5px;padding-left:5px; }");
  addressToolBar->setContentsMargins(0,0,5,0);
#endif

  // Restore window state
  if (wState) {
      qDebug() << "restore window state";
      if (!settings->value("windowState").isValid()) { // don't show dock tree/app as default
          dockTree->hide();
#ifndef NO_APPDOCK
          appDock->hide();
#endif
      }
      restoreState(settings->value("windowState").toByteArray(), 1);
      restoreGeometry(settings->value("windowGeo").toByteArray());
      if (settings->value("windowMax").toBool()) { showMaximized(); }
  }

  // Load info whether use real mime types
  modelList->setRealMimeTypes(settings->value("realMimeTypes", true).toBool());

  // Load information whether hidden files can be displayed
  if (hState) {
      hiddenAct->setChecked(settings->value("hiddenMode", 0).toBool());
      toggleHidden();
  }

  // Remove old bookmarks
  modelBookmarks->removeRows(0, modelBookmarks->rowCount());

  // Load bookmarks
  loadBookmarks();

  // Set bookmarks
  firstRunBookmarks(isFirstRun);
#ifndef NO_UDISKS
  populateMedia();
#endif
  bookmarksList->setModel(modelBookmarks);
  bookmarksList->setResizeMode(QListView::Adjust);
  bookmarksList->setFlow(QListView::TopToBottom);
  bookmarksList->setIconSize(QSize(24,24));

  // Load information whether bookmarks are displayed
  wrapBookmarksAct->setChecked(settings->value("wrapBookmarks", 0).toBool());
  bookmarksList->setWrapping(wrapBookmarksAct->isChecked());

  // Lock information whether layout is locked or not
  lockLayoutAct->setChecked(settings->value("lockLayout", 1).toBool());
  toggleLockLayout();

  // Load zoom settings
  zoom = settings->value("zoom", 48).toInt();
  zoomTree = settings->value("zoomTree", 16).toInt();
  zoomBook = settings->value("zoomBook", 16).toInt();
  zoomList = settings->value("zoomList", 24).toInt();
  zoomDetail = settings->value("zoomDetail", 16).toInt();
  detailTree->setIconSize(QSize(zoomDetail, zoomDetail));
  tree->setIconSize(QSize(zoomTree, zoomTree));
  bookmarksList->setIconSize(QSize(zoomBook, zoomBook));

  // Load information whether thumbnails can be shown
  if (thumbState) {
    thumbsAct->setChecked(settings->value("showThumbs", 1).toBool());
  }

  // Load view mode
  detailAct->setChecked(settings->value("viewMode", 0).toBool());
  iconAct->setChecked(settings->value("iconMode", 1).toBool());
  toggleDetails();

  // Restore header of detail tree
  detailTree->header()->restoreState(settings->value("header").toByteArray());
  detailTree->setSortingEnabled(1);

  // Load sorting information and sort
  currentSortColumn = settings->value("sortBy", 0).toInt();
  currentSortOrder = (Qt::SortOrder) settings->value("sortOrder", 0).toInt();
  switch (currentSortColumn) {
    case 0 : setSortColumn(sortNameAct); break;
    case 1 : setSortColumn(sortSizeAct); break;
    case 3 : setSortColumn(sortDateAct); break;
  }
  setSortOrder(currentSortOrder);
  modelView->sort(currentSortColumn, currentSortOrder);

  // Load terminal command
  term = settings->value("term", "xterm").toString();

  // custom actions
  firstRunCustomActions(isFirstRun);

  // Load information whether tabs can be shown on top
  if (tabState) {
      tabsOnTopAct->setChecked(settings->value("tabsOnTop", 0).toBool());
      tabsOnTop();
  }

  // show/hide buttons
  homeAct->setVisible(settings->value("home_button", true).toBool());
  terminalAct->setVisible(settings->value("terminal_button", true).toBool());

  // path history
  pathHistory = settings->value("pathHistory", true).toBool();

  // path in window title
  showPathInWindowTitle = settings->value("windowTitlePath", true).toBool();
  if (!showPathInWindowTitle) { setWindowTitle(APP_NAME); }
}

void MainWindow::firstRunBookmarks(bool isFirstRun)
{
    if (!isFirstRun) { return; }
    //qDebug() << "first run, setup default bookmarks";
    modelBookmarks->addBookmark(tr("Computer"), "/", "", "computer", "", false, false);
    modelBookmarks->addBookmark(tr("Home"), QDir::homePath(), "", "user-home", "", false, false);
    modelBookmarks->addBookmark(tr("Desktop"), QString("%1/Desktop").arg(QDir::homePath()), "", "user-desktop", "", false, false);
    //modelBookmarks->addBookmark(tr("Documents"), QString("%1/Documents").arg(QDir::homePath()), "", "text-x-generic", "", false, false);
    //modelBookmarks->addBookmark(tr("Downloads"), QString("%1/Dowloads").arg(QDir::homePath()), "", "applications-internet", "", false, false);
    //modelBookmarks->addBookmark(tr("Pictures"), QString("%1/Pictures").arg(QDir::homePath()), "", "image-x-generic", "", false, false);
    //modelBookmarks->addBookmark(tr("Videos"), QString("%1/Videos").arg(QDir::homePath()), "", "video-x-generic", "", false, false);
    //modelBookmarks->addBookmark(tr("Music"), QString("%1/Music").arg(QDir::homePath()), "", "audio-x-generic", "", false, false);
    modelBookmarks->addBookmark(tr("Trash"), QString("%1/.local/share/Trash").arg(QDir::homePath()), "", "user-trash", "", false, false);
    modelBookmarks->addBookmark("", "", "", "", "", false, false);
    writeBookmarks();
}

void MainWindow::loadBookmarks()
{
    //qDebug() << "load bookmarks";
    settings->beginGroup("bookmarks");
    foreach (QString key,settings->childKeys()) {
      QStringList temp(settings->value(key).toStringList());
      modelBookmarks->addBookmark(temp[0], temp[1], temp[2], temp.last(), "", false, false);
    }
    settings->endGroup();
}

void MainWindow::writeBookmarks()
{
    //qDebug() << "write bookmarks";
    settings->remove("bookmarks");
    settings->beginGroup("bookmarks");
    for (int i = 0; i < modelBookmarks->rowCount(); i++) {
      if (modelBookmarks->item(i)->data(MEDIA_MODEL).toBool()) { continue; } // ignore media devices
      QStringList temp;
      temp << modelBookmarks->item(i)->text()
           << modelBookmarks->item(i)->data(BOOKMARK_PATH).toString()
           << modelBookmarks->item(i)->data(BOOKMARKS_AUTO).toString()
           << modelBookmarks->item(i)->data(BOOKMARK_ICON).toString();
      settings->setValue(QString(i),temp);
    }
    settings->endGroup();
}

void MainWindow::handleBookmarksChanged()
{
    //qDebug() << "bookmarks changed, save";
    QTimer::singleShot(1000, this, SLOT(writeBookmarks()));
}

void MainWindow::firstRunCustomActions(bool isFirstRun)
{
    if (!isFirstRun) { return; }
    settings->beginGroup("customActions");
    int childs = settings->childKeys().size();
    if (childs>0) { return; }
    QStringList action1;
    action1 << "gz,bz2,xz,tar" << "Extract here ..." << "package-x-generic" << "tar xvf %f";
    QStringList action2;
    action2 << "*" << "Compress to tar.gz" << "package-x-generic" << "tar cvvzf %n.tar.gz %f";
    QStringList action3;
    action3 << "*" << "Compress to tar.bz2" << "package-x-generic" << "tar cvvjf %n.tar.bz2 %f";
    QStringList action4;
    action4 << "*" << "Compress to tar.xz" << "package-x-generic" << "tar cvvJf %n.tar.bz2 %f";
    settings->setValue(QString(1), action1);
    settings->setValue(QString(2), action2);
    settings->setValue(QString(3), action3);
    settings->setValue(QString(4), action4);
    settings->endGroup();
    settings->sync();
}
//---------------------------------------------------------------------------

/**
 * @brief Close event
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Save settings
    writeSettings();

    modelList->cacheInfo();
    event->accept();
}
//---------------------------------------------------------------------------

/**
 * @brief Closes main window
 */
void MainWindow::exitAction() {
  close();
}
//---------------------------------------------------------------------------

void MainWindow::treeSelectionChanged(QModelIndex current, QModelIndex previous)
{
    Q_UNUSED(previous)

    QFileInfo name = modelList->fileInfo(modelTree->mapToSource(current));
    if (!name.exists()) { return; }

    curIndex = name;
    if (showPathInWindowTitle) {
        if (curIndex.fileName().isEmpty()) { setWindowTitle(curIndex.absolutePath()); }
        else { setWindowTitle(curIndex.fileName()); }
    } else {
        setWindowTitle(APP_NAME);
    }

    if (tree->hasFocus() && QApplication::mouseButtons() == Qt::MidButton) {
        listItemPressed(modelView->mapFromSource(modelList->index(name.filePath())));
        tabs->setCurrentIndex(tabs->count() - 1);
        if (currentView == 2) { detailTree->setFocus(Qt::TabFocusReason); }
        else { list->setFocus(Qt::TabFocusReason); }
    }

    if (curIndex.filePath() != pathEdit->itemText(0)) {
        if (tabs->count() && pathHistory) { tabs->addHistory(curIndex.filePath()); }
        if (!pathHistory && pathEdit->count()>0) { pathEdit->clear(); }
        pathEdit->insertItem(0,curIndex.filePath());
        pathEdit->setCurrentIndex(0);
    }

    if (!bookmarksList->hasFocus()) { bookmarksList->clearSelection(); }

    if (modelList->setRootPath(name.filePath())) { modelView->invalidate(); }

    //////
    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(name.filePath()));

    if (currentView == 2) { detailTree->setRootIndex(baseIndex); }
    else { list->setRootIndex(baseIndex); }

    if(tabs->count()) {
        QString tabText = curIndex.fileName();
        if (tabText.isEmpty()) { tabText = "/"; }
        tabs->setTabText(tabs->currentIndex(),tabText);
        tabs->setTabData(tabs->currentIndex(),curIndex.filePath());
        tabs->setIcon(tabs->currentIndex());
    }

    if(backIndex.isValid()) {
        listSelectionModel->setCurrentIndex(modelView->mapFromSource(backIndex),QItemSelectionModel::ClearAndSelect);
        if (currentView == 2) { detailTree->scrollTo(modelView->mapFromSource(backIndex)); }
        else { list->scrollTo(modelView->mapFromSource(backIndex)); }
    } else {
        listSelectionModel->blockSignals(1);
        listSelectionModel->clear();
    }

    listSelectionModel->blockSignals(0);
    QTimer::singleShot(30,this,SLOT(dirLoaded()));
}

//---------------------------------------------------------------------------
void MainWindow::dirLoaded()
{
    qDebug() << "dirLoaded";
    if (backIndex.isValid()) {
        backIndex = QModelIndex();
        return;
    }

    qint64 bytes = 0;
    QModelIndexList items;
    bool includeHidden = hiddenAct->isChecked();

    for (int x = 0; x < modelList->rowCount(modelList->index(pathEdit->currentText())); ++x) {
        items.append(modelList->index(x,0,modelList->index(pathEdit->currentText())));
    }


    foreach (QModelIndex theItem,items) {
        if (includeHidden || !modelList->fileInfo(theItem).isHidden()) {
            bytes = bytes + modelList->size(theItem);
        } else { items.removeOne(theItem); }
    }

    QString total;

    if (!bytes) { total = ""; }
    else { total = formatSize(bytes); }

    statusName->clear();
    statusSize->setText(QString("%1 items").arg(items.count()));
    statusDate->setText(QString("%1").arg(total));

    if (thumbsAct->isChecked()) { QtConcurrent::run(modelList,&myModel::loadThumbs,items); }
    updateGrid();
}

//---------------------------------------------------------------------------
void MainWindow::thumbUpdate(QModelIndex index)
{
    qDebug() << "thumbUpdate";
    if (currentView == 2) { detailTree->update(modelView->mapFromSource(index)); }
    else { list->update(modelView->mapFromSource(index)); }
}

//---------------------------------------------------------------------------
void MainWindow::listSelectionChanged(const QItemSelection selected, const QItemSelection deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QModelIndexList items;

    if (listSelectionModel->selectedRows(0).count()) { items = listSelectionModel->selectedRows(0); }
    else { items = listSelectionModel->selectedIndexes(); }

    statusSize->clear();
    statusDate->clear();
    statusName->clear();

    if(items.count() == 0) {
        curIndex = pathEdit->itemText(0);
        return;
    }

    if (QApplication::focusWidget() != bookmarksList) { bookmarksList->clearSelection(); }

    curIndex = modelList->fileInfo(modelView->mapToSource(listSelectionModel->currentIndex()));

    qint64 bytes = 0;
    int folders = 0;
    int files = 0;

    foreach(QModelIndex theItem,items) {
        if (modelList->isDir(modelView->mapToSource(theItem))) { folders++; }
        else { files++; }
        bytes = bytes + modelList->size(modelView->mapToSource(theItem));
    }

    QString total,name;

    if (!bytes) { total = ""; }
    else { total = formatSize(bytes); }

    if (items.count() == 1) {
        QFileInfo file(modelList->filePath(modelView->mapToSource(items.at(0))));

        name = file.fileName();
        if (file.isSymLink()) { name = "Link --> " + file.symLinkTarget(); }

        statusName->setText(name + "   ");
        statusSize->setText(QString("%1   ").arg(total));
        statusDate->setText(QString("%1").arg(file.lastModified().toString(Qt::SystemLocaleShortDate)));
    }
    else {
        statusName->setText(total + "   ");
        if (files) { statusSize->setText(QString("%1 files  ").arg(files)); }
        if (folders) { statusDate->setText(QString("%1 folders").arg(folders)); }
    }
}

//---------------------------------------------------------------------------
QString formatSize(qint64 num)
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if (num >= tb) { total = QString("%1TB").arg(QString::number(qreal(num) / tb, 'f', 2)); }
    else if (num >= gb) { total = QString("%1GB").arg(QString::number(qreal(num) / gb, 'f', 2)); }
    else if (num >= mb) { total = QString("%1MB").arg(QString::number(qreal(num) / mb, 'f', 1)); }
    else if (num >= kb) { total = QString("%1KB").arg(QString::number(qreal(num) / kb,'f', 1)); }
    else { total = QString("%1 bytes").arg(num); }

    return total;
}

//---------------------------------------------------------------------------
void MainWindow::listItemClicked(QModelIndex current)
{
    if (modelList->filePath(modelView->mapToSource(current)) == pathEdit->currentText()) { return; }

    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if (mods == Qt::ControlModifier || mods == Qt::ShiftModifier) { return; }
    if (modelList->isDir(modelView->mapToSource(current))) {
        tree->setCurrentIndex(modelTree->mapFromSource(modelView->mapToSource(current)));
    }
}

//---------------------------------------------------------------------------
void MainWindow::listItemPressed(QModelIndex current)
{
    //middle-click -> open new tab
    //ctrl+middle-click -> open new instance

    if (QApplication::mouseButtons() == Qt::MidButton) {
        if(modelList->isDir(modelView->mapToSource(current))) {
            if (QApplication::keyboardModifiers() == Qt::ControlModifier) { openFile(); }
            else { addTab(modelList->filePath(modelView->mapToSource(current))); }
        } else { openFile(); }
    }
}

//---------------------------------------------------------------------------
void MainWindow::openTab()
{
    if(curIndex.isDir()) {
        addTab(curIndex.filePath());
    }
}

//---------------------------------------------------------------------------
int MainWindow::addTab(QString path)
{
    if (tabs->count() == 0) { tabs->addNewTab(pathEdit->currentText(),currentView); }
    return tabs->addNewTab(path,currentView);
}

//---------------------------------------------------------------------------
void MainWindow::tabsOnTop()
{
    if(tabsOnTopAct->isChecked()) {
        mainLayout->setDirection(QBoxLayout::BottomToTop);
        tabs->setShape(QTabBar::RoundedNorth);
    } else {
        mainLayout->setDirection(QBoxLayout::TopToBottom);
        tabs->setShape(QTabBar::RoundedSouth);
    }
}

//---------------------------------------------------------------------------
void MainWindow::tabChanged(int index)
{
    if (tabs->count() == 0) { return; }

    pathEdit->clear();
    pathEdit->addItems(*tabs->getHistory(index));

    int type = tabs->getType(index);
    if(currentView != type) {
        if (type == 2) { detailAct->setChecked(1); }
        else { detailAct->setChecked(0); }
        if (type == 1) { iconAct->setChecked(1); }
        else { iconAct->setChecked(0); }
        toggleDetails();
    }

    if(!tabs->tabData(index).toString().isEmpty()) {
        tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(tabs->tabData(index).toString())));
    }
}

void MainWindow::newWindow()
{
    if (settings->value("clearCache").toBool()) {
        settings->setValue("clearCache", false); // we don't want the new window to clear our existing cache
    }
    writeSettings();
    QProcess::startDetached(qApp->applicationFilePath());
}


//---------------------------------------------------------------------------

/**
 * @brief Doubleclick on icon/launcher
 * @param current
 */
void MainWindow::listDoubleClicked(QModelIndex current) {
  Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
  if (mods == Qt::ControlModifier || mods == Qt::ShiftModifier) {
    return;
  }
  if (modelList->isDir(modelView->mapToSource(current))) {
    QModelIndex i = modelView->mapToSource(current);
    tree->setCurrentIndex(modelTree->mapFromSource(i));
  } else {
    executeFile(current, 0);
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Reaction for change of path edit (location edit)
 * @param path
 */
void MainWindow::pathEditChanged(QString path) {
  QString info = path;
  if (!QFileInfo(path).exists()) { return; }
  info.replace("~",QDir::homePath());
  tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(info)));
}
//---------------------------------------------------------------------------

/**
 * @brief Reaction for change of clippboard content
 */
void MainWindow::clipboardChanged() {
  if (QApplication::clipboard()->mimeData()->hasUrls()) {
    pasteAct->setEnabled(true);
  } else {
    modelList->clearCutItems();
    pasteAct->setEnabled(false);
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Pastes from clipboard
 */
void MainWindow::pasteClipboard() {
  QString newPath;
  QStringList cutList;

  if (curIndex.isDir()) { newPath = curIndex.filePath(); }
  else { newPath = pathEdit->itemText(0); }

  // Check list of files that are to be cut
  QFile tempFile(QDir::tempPath() + "/" + APP + ".temp");
  if (tempFile.exists()) {
    tempFile.open(QIODevice::ReadOnly);
    QDataStream out(&tempFile);
    out >> cutList;
    tempFile.close();
  }
  pasteLauncher(QApplication::clipboard()->mimeData(), newPath, cutList);
}
//---------------------------------------------------------------------------

/**
 * @brief Drags data to the new location
 * @param data data to be pasted
 * @param newPath path of new location
 * @param dragMode mode of dragging
 */
void MainWindow::dragLauncher(const QMimeData *data, const QString &newPath,
                              Common::DragMode dragMode) {

  // Retrieve urls (paths) of data
  QList<QUrl> files = data->urls();

  // get original path
  QStringList getOldPath = files.at(0).toLocalFile().split("/", QString::SkipEmptyParts);
  QString oldPath;
  for (int i=0;i<getOldPath.size()-1;++i) { oldPath.append(QString("/%1").arg(getOldPath.at(i))); }
  QString oldDevice = Common::getDeviceForDir(oldPath);
  QString newDevice = Common::getDeviceForDir(newPath);

  qDebug() << "oldpath:" << oldDevice << oldPath;
  qDebug() << "newpath:" << newDevice << newPath;

  QString extraText;
  Common::DragMode currentDragMode = dragMode;
  if (oldDevice != newDevice) {
      extraText = QString(tr("Source and destination is on a different storage."));
      currentDragMode = Common::DM_UNKNOWN;
  }

  // If drag mode is unknown then ask what to do
  if (currentDragMode == Common::DM_UNKNOWN) {
    QMessageBox box;
    box.setWindowTitle(tr("Select file action"));
    box.setWindowIcon(QIcon::fromTheme("folder"));
    box.setIconPixmap(QIcon::fromTheme("dialog-information").pixmap(QSize(32, 32)));
    box.setText(tr("<h3>What do you want to do?</h3>"));
    if (!extraText.isEmpty()) {
        box.setText(QString("%1<p>%2</p>").arg(box.text()).arg(extraText));
    }
    QAbstractButton *move = box.addButton(tr("Move here"), QMessageBox::ActionRole);
    QAbstractButton *copy = box.addButton(tr("Copy here"), QMessageBox::ActionRole);
    QAbstractButton *link = box.addButton(tr("Link here"), QMessageBox::ActionRole);
    QAbstractButton *canc = box.addButton(QMessageBox::Cancel);
    move->setIcon(QIcon::fromTheme("edit-cut"));
    copy->setIcon(QIcon::fromTheme("edit-copy"));
    link->setIcon(QIcon::fromTheme("insert-link"));
    canc->setIcon(QIcon::fromTheme("edit-delete"));

    box.exec();
    if (box.clickedButton() == move) {
      dragMode = Common::DM_MOVE;
    } else if (box.clickedButton() == copy) {
      dragMode = Common::DM_COPY;
    } else if (box.clickedButton() == link) {
      dragMode = Common::DM_LINK;
    } else if (box.clickedButton() == canc) {
      return;
    }
  }

  // If moving is enabled, cut files from the original location
  QStringList cutList;
  if (currentDragMode == Common::DM_MOVE) {
    foreach (QUrl item, files) {
      cutList.append(item.path());
    }
  }

  // Paste launcher (this method has to be called instead of that with 'data'
  // parameter, because that 'data' can timeout)
  pasteLauncher(files, newPath, cutList, dragMode == Common::DM_LINK);
}
//---------------------------------------------------------------------------

/**
 * @brief Pastes data to the new location
 * @param data data to be pasted
 * @param newPath path of new location
 * @param cutList list of items to remove
 */
void MainWindow::pasteLauncher(const QMimeData *data, const QString &newPath,
                               const QStringList &cutList) {
  QList<QUrl> files = data->urls();
  if (files.isEmpty()) { return; }
  pasteLauncher(files, newPath, cutList);
}
//---------------------------------------------------------------------------

/**
 * @brief Pastes files to the new path
 * @param files list of files
 * @param newPath new path
 * @param cutList files to remove from original path
 * @param link true if link should be created (default value = false)
 */
void MainWindow::pasteLauncher(const QList<QUrl> &files, const QString &newPath,
                               const QStringList &cutList, bool link) {

  // File no longer exists?
  if (!QFile(files.at(0).path()).exists()) {
    QString msg = tr("File '%1' no longer exists!").arg(files.at(0).path());
    QMessageBox::information(this, tr("No paste for you!"), msg);
    pasteAct->setEnabled(false);
    return;
  }

  // Temporary variables
  int replace = 0;
  QStringList completeList;
  QString baseName = QFileInfo(files.at(0).toLocalFile()).path();

  // Only if not in same directory, otherwise we will do 'Copy(x) of'
  if (newPath != baseName) {

    foreach (QUrl file, files) {

      // Merge or replace?
      QFileInfo temp(file.toLocalFile());

      if (temp.isDir() && QFileInfo(newPath + QDir::separator() + temp.fileName()).exists()) {
        QString msg = QString("<b>%1</b><p>Already exists!<p>What do you want to do?").arg(newPath + QDir::separator() + temp.fileName());
        QMessageBox message(QMessageBox::Question, tr("Existing folder"), msg, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        message.button(QMessageBox::Yes)->setText(tr("Merge"));
        message.button(QMessageBox::No)->setText(tr("Replace"));

        int merge = message.exec();
        if (merge == QMessageBox::Cancel) { return; }
        if (merge == QMessageBox::Yes) {
          FileUtils::recurseFolder(temp.filePath(), temp.fileName(), &completeList);
        } else {
          FileUtils::removeRecurse(newPath, temp.fileName());
        }
      } else completeList.append(temp.fileName());
    }

    // Ask whether replace files if files with same name already exist in
    // destination directory
    foreach (QString file, completeList) {
      QFileInfo temp(newPath + QDir::separator() + file);
      if (temp.exists()) {
        QFileInfo orig(baseName + QDir::separator() + file);
        if (replace != QMessageBox::YesToAll && replace != QMessageBox::NoToAll) {
          // TODO: error dispalys only at once
          replace = showReplaceMsgBox(temp, orig);
        }
        if (replace == QMessageBox::Cancel) {
          return;
        }
        if (replace == QMessageBox::Yes || replace == QMessageBox::YesToAll) {
          QFile(temp.filePath()).remove();
        }
      }
    }
  }

  // If only links should be created, create them and exit
  if (link) {
    linkFiles(files, newPath);
    return;
  }

  // Copy/move files
  QString title = cutList.count() == 0 ? tr("Copying...") : tr("Moving...");
  progress = new myProgressDialog(title);
  connect(this, SIGNAL(updateCopyProgress(qint64, qint64, QString)), progress, SLOT(update(qint64, qint64, QString)));
  listSelectionModel->clear();
  QtConcurrent::run(this, &MainWindow::pasteFiles, files, newPath, cutList);
}
//---------------------------------------------------------------------------

/**
 * @brief Asks user whether replace file 'f1' with another file 'f2'
 * @param f1 file to be replaced with f2
 * @param f2 file to replace f1
 * @return result
 */
int MainWindow::showReplaceMsgBox(const QFileInfo &f1, const QFileInfo &f2) {

  // Create message
  QString t = tr("<h3>Do you want to replace?</h3><p><b>%1</p><p>Modified: %2<br>"
                 "Size: %3 bytes</p><p>with:<p><b>%4</p><p>Modified: %5"
                 "<br>Size: %6 bytes</p>");

  // Populate message with data
  t = t.arg(f1.filePath()).arg(f1.lastModified().toString()).arg(f1.size())
       .arg(f2.filePath()).arg(f2.lastModified().toString()).arg(f2.size());

  // Show message
  return QMessageBox::question(0, tr("Replace"), t, QMessageBox::Yes
                               | QMessageBox::YesToAll | QMessageBox::No
                               | QMessageBox::NoToAll | QMessageBox::Cancel);
}
//---------------------------------------------------------------------------

void MainWindow::progressFinished(int ret,QStringList newFiles)
{
    if (progress != 0) {
        progress->close();
        delete progress;
        progress = 0;
    }

    if (newFiles.count()) {
        disconnect(listSelectionModel,SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),this,SLOT(listSelectionChanged(const QItemSelection, const QItemSelection)));

        qApp->processEvents();              //make sure notifier has added new files to the model

        if (QFileInfo(newFiles.first()).path() == pathEdit->currentText()) { // highlight new files if visible
            foreach(QString item, newFiles) {
                listSelectionModel->select(modelView->mapFromSource(modelList->index(item)),QItemSelectionModel::Select);
            }
        }

        connect(listSelectionModel,SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),this,SLOT(listSelectionChanged(const QItemSelection, const QItemSelection)));
        curIndex.setFile(newFiles.first());

        if (currentView == 2) { detailTree->scrollTo(modelView->mapFromSource(modelList->index(newFiles.first())),QAbstractItemView::EnsureVisible); }
        else { list->scrollTo(modelView->mapFromSource(modelList->index(newFiles.first())),QAbstractItemView::EnsureVisible); }

        if (QFile(QDir::tempPath() + QString("/%1.temp").arg(APP)).exists()) { QApplication::clipboard()->clear(); }

        clearCutItems();
    }

    if (ret == 1) { QMessageBox::information(this,tr("Failed"),tr("Paste failed...do you have write permissions?")); }
    if (ret == 2) { QMessageBox::warning(this,tr("Too big!"),tr("There is not enough space on the destination storage!")); }
}

//---------------------------------------------------------------------------

void MainWindow::folderPropertiesLauncher()
{
    QModelIndexList selList;
    if (focusWidget() == bookmarksList) {
        selList.append(modelView->mapFromSource(modelList->index(bookmarksList->currentIndex().data(BOOKMARK_PATH).toString())));
    } else if (focusWidget() == list || focusWidget() == detailTree) {
        if (listSelectionModel->selectedRows(0).count()) { selList = listSelectionModel->selectedRows(0); }
        else { selList = listSelectionModel->selectedIndexes(); }
    }

    if (selList.count() == 0) { selList << modelView->mapFromSource(modelList->index(pathEdit->currentText())); }

    QStringList paths;

    foreach (QModelIndex item, selList) {
        paths.append(modelList->filePath(modelView->mapToSource(item)));
    }

    properties = new PropertiesDialog(paths, modelList);
    connect(properties,SIGNAL(propertiesUpdated()),this,SLOT(clearCutItems()));
}

//---------------------------------------------------------------------------

/**
 * @brief Writes settings into config file
 */
void MainWindow::writeSettings() {

  // Write general settings
  settings->setValue("viewMode", stackWidget->currentIndex());
  settings->setValue("iconMode", iconAct->isChecked());
  settings->setValue("zoom", zoom);
  settings->setValue("zoomTree", zoomTree);
  settings->setValue("zoomBook", zoomBook);
  settings->setValue("zoomList", zoomList);
  settings->setValue("zoomDetail", zoomDetail);
  settings->setValue("sortBy", currentSortColumn);
  settings->setValue("sortOrder", currentSortOrder);
  settings->setValue("showThumbs", thumbsAct->isChecked());
  settings->setValue("hiddenMode", hiddenAct->isChecked());
  settings->setValue("lockLayout", lockLayoutAct->isChecked());
  settings->setValue("tabsOnTop", tabsOnTopAct->isChecked());
  settings->setValue("windowState", saveState(1));
  settings->setValue("windowGeo", saveGeometry());
  settings->setValue("windowMax", isMaximized());
  settings->setValue("header", detailTree->header()->saveState());
  settings->setValue("realMimeTypes",  modelList->isRealMimeTypes());

  // Write bookmarks
  writeBookmarks();
}
//---------------------------------------------------------------------------

/**
 * @brief Display popup menu
 * @param event
 */
void MainWindow::contextMenuEvent(QContextMenuEvent * event) {

  // Retreive widget under mouse
  QMenu *popup;
  QWidget *widget = childAt(event->pos());

  // Create popup for tab or for status bar
  if (widget == tabs) {
    popup = new QMenu(this);
    popup->addAction(closeTabAct);
    popup->exec(event->globalPos());
    return;
  } else if (widget == status) {
    popup = createPopupMenu();
    popup->addSeparator();
    popup->addAction(lockLayoutAct);
    popup->exec(event->globalPos());
    return;
  }
#ifndef NO_APPDOCK
  else if (focusWidget() == appDock->widget()) { return; }
#endif

  // Continue with poups for folders and files
  QList<QAction*> actions;
  popup = new QMenu(this);

  bool isMedia = false;

  if (focusWidget() == list || focusWidget() == detailTree) {

    // Clear selection in bookmarks
    bookmarksList->clearSelection();

    // Could be file or folder
    if (listSelectionModel->hasSelection()) {

      // Get index of source model
      curIndex = modelList->filePath(modelView->mapToSource(listSelectionModel->currentIndex()));

      // File
      if (!curIndex.isDir()) {
        QString type = modelList->getMimeType(modelList->index(curIndex.filePath()));
        //qDebug() << "type" << type;

        // Add custom actions to the list of actions
        //qDebug() << "add custom actions";
        QHashIterator<QString, QAction*> i(*customActManager->getActions());
        while (i.hasNext()) {
          i.next();
          //qDebug() << "custom action" << i.key() << i.value();
          if (type.contains(i.key())) { actions.append(i.value()); }
        }

        // Add run action or open with default application action
        if (curIndex.isExecutable()) {
          popup->addAction(runAct);
        } else {
          popup->addAction(openAct);
        }

        // Add open action
        /*foreach (QAction* action, actions) {
          if (action->text() == "Open") {
            popup->addAction(action);
            break;
          }
        }*/

        // Add open with menu
        popup->addMenu(createOpenWithMenu());

        //if (popup->actions().count() == 0) popup->addAction(openAct);

        // Add custom actions that are associated only with this file type
        if (!actions.isEmpty()) {
          popup->addSeparator();
          popup->addActions(actions);
          popup->addSeparator();
        }

        // Add menus
        QHashIterator<QString, QMenu*> m(*customActManager->getMenus());
        while (m.hasNext()) {
          m.next();
          if (type.contains(m.key())) { popup->addMenu(m.value()); }
        }

        // Add cut/copy/paste/rename actions
        popup->addSeparator();
        popup->addAction(cutAct);
        popup->addAction(copyAct);
        popup->addAction(pasteAct);
        popup->addSeparator();
        popup->addAction(renameAct);
        popup->addSeparator();

        // Add custom actions that are associated with all file types
        foreach (QMenu* parent, customActManager->getMenus()->values("*")) {
          popup->addMenu(parent);
        }
        actions = (customActManager->getActions()->values("*"));
        popup->addActions(actions);
        if (customActManager->getActionList()->size()>0) {
            popup->addSeparator();
        }
        if (modelList->getRootPath() != trashDir) {
            popup->addAction(trashAct);
        }
        popup->addAction(deleteAct);
        popup->addSeparator();
        actions = customActManager->getActions()->values(curIndex.path());    //children of $parent
        if (actions.count()) {
          popup->addActions(actions);
          popup->addSeparator();
        }
      }
      // Folder/directory
      else {
        popup->addAction(openAct);
        popup->addSeparator();
        popup->addAction(cutAct);
        popup->addAction(copyAct);
        popup->addAction(pasteAct);
        popup->addSeparator();
        popup->addAction(renameAct);
        popup->addSeparator();

        foreach (QMenu* parent, customActManager->getMenus()->values("*")) {
          popup->addMenu(parent);
        }

        actions = customActManager->getActions()->values("*");
        popup->addActions(actions);
        if (modelList->getRootPath() != trashDir) {
            popup->addAction(trashAct);
        }
        popup->addAction(deleteAct);
        popup->addSeparator();

        foreach (QMenu* parent, customActManager->getMenus()->values("folder")) {
          popup->addMenu(parent);
        }
        actions = customActManager->getActions()->values(curIndex.fileName());   // specific folder
        actions.append(customActManager->getActions()->values(curIndex.path())); // children of $parent
        actions.append(customActManager->getActions()->values("folder"));        // all folders
        if (actions.count()) {
          popup->addActions(actions);
          popup->addSeparator();
        }
      }
      popup->addAction(folderPropertiesAct);
    }
    // Whitespace
    else {
      popup->addAction(newDirAct);
      popup->addAction(newFileAct);
      popup->addSeparator();
      if (pasteAct->isEnabled()) {
        popup->addAction(pasteAct);
        popup->addSeparator();
      }
      popup->addAction(addBookmarkAct);
      popup->addSeparator();

      foreach (QMenu* parent, customActManager->getMenus()->values("folder")) {
        popup->addMenu(parent);
      }
      actions = customActManager->getActions()->values(curIndex.fileName());
      actions.append(customActManager->getActions()->values("folder"));
      if (actions.count()) {
        foreach (QAction*action, actions) {
          popup->addAction(action);
        }
        popup->addSeparator();
      }
      popup->addAction(folderPropertiesAct);
    }
  }
  // Tree or bookmarks
  else {
    if (focusWidget() == bookmarksList) {
      listSelectionModel->clearSelection();
      if (bookmarksList->indexAt(bookmarksList->mapFromGlobal(event->globalPos())).isValid()) {
        curIndex = bookmarksList->currentIndex().data(BOOKMARK_PATH).toString();
        isMedia = bookmarksList->currentIndex().data(MEDIA_MODEL).toBool();
        if (!isMedia) {
            popup->addAction(delBookmarkAct);
            popup->addAction(editBookmarkAct);	//icon
        } else {
            // media actions
#ifndef NO_UDISKS
            QString mediaPath = bookmarksList->currentIndex().data(MEDIA_PATH).toString();
            if (!mediaPath.isEmpty()) {
                if (!disks->devices[mediaPath]->mountpoint.isEmpty()) { // mounted
                    popup->addAction(mediaUnmountAct);
                } else { // unmounted
                    if (disks->devices[mediaPath]->isOptical) { popup->addAction(mediaEjectAct); }
                }
            }
#endif
        }
      } else {
        bookmarksList->clearSelection();
        popup->addAction(addSeparatorAct);	//seperator
        popup->addAction(wrapBookmarksAct);
      }
      popup->addSeparator();
    } else {
      bookmarksList->clearSelection();
      popup->addAction(newDirAct);
      popup->addAction(newFileAct);
      popup->addAction(newWinAct);
      popup->addAction(openTabAct);
      popup->addSeparator();
      popup->addAction(cutAct);
      popup->addAction(copyAct);
      popup->addAction(pasteAct);
      popup->addSeparator();
      popup->addAction(renameAct);
      popup->addSeparator();
      if (modelList->getRootPath() != trashDir) {
        popup->addAction(trashAct);
      }
      popup->addAction(deleteAct);
    }
    popup->addSeparator();

    foreach (QMenu* parent, customActManager->getMenus()->values("folder")) {
      popup->addMenu(parent);
    }
    actions = customActManager->getActions()->values(curIndex.fileName());
    actions.append(customActManager->getActions()->values(curIndex.path()));
    actions.append(customActManager->getActions()->values("folder"));
    if (actions.count()) {
      foreach (QAction*action, actions) { popup->addAction(action); }
      popup->addSeparator();
    }
    if (!isMedia) { popup->addAction(folderPropertiesAct); }
  }

  popup->exec(event->globalPos());
  delete popup;
}
//---------------------------------------------------------------------------

/**
 * @brief Creates menu for opening file in selected application
 * @return menu
 */
QMenu* MainWindow::createOpenWithMenu() {

    //qDebug() << "open with";
  // Add open with functionality ...
  QMenu *openMenu = new QMenu(tr("Open with"));

  // Select action
  QAction *selectAppAct = new QAction(tr("Select..."), openMenu);
  selectAppAct->setStatusTip(tr("Select application for opening the file"));
  //selectAppAct->setIcon(actionIcons->at(18));
  connect(selectAppAct, SIGNAL(triggered()), this, SLOT(selectApp()));

  // Load default applications for current mime
  QString mime = mimeUtils->getMimeType(curIndex.filePath());
  QStringList appNames = mimeUtils->getDefault(mime);
  if (appNames.size()==1 && appNames.at(0).isEmpty() && mime.startsWith("text/")) {
      qDebug() << "get fallback apps for text/plain";
      appNames = mimeUtils->getDefault("text/plain");
  }

  qDebug() << mime << appNames;

  // Create actions for opening
  QList<QAction*> defaultApps;
  foreach (QString appName, appNames) {
    // Skip empty app name
    if (appName.isEmpty()) { continue; }

    // find .desktop
    QString appDesktopFile = Common::findApplication(qApp->applicationFilePath(), appName);
    if (appDesktopFile.isEmpty()) { continue; }

    // Load desktop file for application
    DesktopFile df = DesktopFile(appDesktopFile);

    // Create action
    QAction* action = new QAction(df.getName(), openMenu);
    action->setData(/*df.getExec()*/appDesktopFile);
    action->setIcon(FileUtils::searchAppIcon(df));
    defaultApps.append(action);

    // TODO: icon and connect
    connect(action, SIGNAL(triggered()), SLOT(openInApp()));

    // Add action to menu
    openMenu->addAction(action);
  }

  // Add open action to menu
  if (!defaultApps.isEmpty()) {
    openMenu->addSeparator();
  }
  openMenu->addAction(selectAppAct);
  return openMenu;
}
//---------------------------------------------------------------------------

/**
 * @brief Selects application for opening file
 */
void MainWindow::selectApp() {
  // Select application in the dialog
  ApplicationDialog *dialog = new ApplicationDialog(this);
  if (dialog->exec()) {
    if (dialog->getCurrentLauncher().compare("") != 0) {
      QString appName = dialog->getCurrentLauncher() + ".desktop";
      QString desktop = Common::findApplication(qApp->applicationFilePath(), appName);
      if (desktop.isEmpty()) { return; }
      DesktopFile df = DesktopFile(desktop);
      mimeUtils->openInApp(df.getExec(), curIndex, df.isTerminal()?term:"");
    }
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Opens file in application
 */
void MainWindow::openInApp() {
  QAction* action = dynamic_cast<QAction*>(sender());
  if (action) {
    DesktopFile df = DesktopFile(action->data().toString());
    if (df.getExec().isEmpty()) { return; }
    mimeUtils->openInApp(df.getExec(), curIndex, df.isTerminal()?term:"");
  }
}

void MainWindow::updateGrid()
{
    if (!iconAct->isChecked()) { return; }
    QFontMetrics fm = fontMetrics();
    int textWidth = fm.averageCharWidth() * 13;
    int textHeight = fm.lineSpacing() * 3;
    QSize grid;
    grid.setWidth(qMax(zoom, textWidth) + 4);
    grid.setHeight(zoom+ textHeight + 4);

    QModelIndexList items;
    for (int x = 0; x < modelList->rowCount(modelList->index(pathEdit->currentText())); ++x) {
        items.append(modelList->index(x,0,modelList->index(pathEdit->currentText())));
    }
    foreach (QModelIndex theItem,items) {
        QString filename = modelList->fileName(theItem);
        QRect txtRect(0, 0, grid.width(), grid.height());
        QSize txtsize = fm.boundingRect(txtRect, Qt::AlignCenter|Qt::TextWrapAnywhere, filename).size();
        int width = txtsize.width()+8;
        if (width<zoom) { width = zoom+8; }
        if (width>grid.width()) { grid.setWidth(width); }
        if (txtsize.height()+zoom+8>grid.height()) { grid.setHeight(txtsize.height()+zoom+8); }
        //qDebug() << filename << txtsize;
    }

    if (list->gridSize() != grid) {
        qDebug() << "SET GRID" << grid;
        list->setGridSize(grid);
    }
}

//---------------------------------------------------------------------------
/**
 * @brief media support
 */
#ifndef NO_UDISKS
void MainWindow::populateMedia()
{
    QMapIterator<QString, Device*> device(disks->devices);
    while (device.hasNext()) {
        device.next();
        if (mediaBookmarkExists(device.value()->path)>-1) { continue; }
        if ((device.value()->isOptical && !device.value()->hasMedia)
                || (!device.value()->isOptical && !device.value()->isRemovable)
                || (!device.value()->isOptical && !device.value()->hasPartition)) { continue; }
        modelBookmarks->addBookmark(QString("%1 (%2)").arg(device.value()->name).arg(device.value()->dev),
                                    device.value()->mountpoint,
                                    "",
                                    device.value()->isOptical?"drive-optical":"drive-removable-media",
                                    device.value()->path,
                                    true,
                                    false);
    }
}

void MainWindow::handleMediaMountpointChanged(QString path, QString mountpoint)
{
    Q_UNUSED(mountpoint)
    if (path.isEmpty()) { return; }
    for (int i = 0; i < modelBookmarks->rowCount(); i++) {
        if (modelBookmarks->item(i)->data(MEDIA_MODEL).toBool() && modelBookmarks->item(i)->data(MEDIA_PATH).toString() == path) {
            modelBookmarks->item(i)->setData(disks->devices[path]->mountpoint, BOOKMARK_PATH);
        }
    }
}

int MainWindow::mediaBookmarkExists(QString path)
{
    if (path.isEmpty()) { return -1; }
    for (int i = 0; i < modelBookmarks->rowCount(); ++i) {
        if (modelBookmarks->item(i)->data(MEDIA_MODEL).toBool()
                && modelBookmarks->item(i)->data(MEDIA_PATH).toString() == path) { return i; }
    }
    return -1;
}

void MainWindow::handleMediaAdded(QString path)
{
    Q_UNUSED(path)
    populateMedia();
}

void MainWindow::handleMediaRemoved(QString path)
{
    int bookmark = mediaBookmarkExists(path);
    if (bookmark>-1) { modelBookmarks->removeRow(bookmark); }
}

void MainWindow::handleMediaChanged(QString path, bool present)
{
    //qDebug() << "changed" << path << present;
    if (path.isEmpty()) { return; }
    if (disks->devices[path]->isOptical && !present && mediaBookmarkExists(path)>-1) {
        handleMediaRemoved(path);
    } else if (disks->devices[path]->isOptical && present && mediaBookmarkExists(path)==-1) {
        handleMediaAdded(path);
    }
}

void MainWindow::handleMediaUnmount()
{
    //qDebug() << "handle media unmount";
    QStandardItem *item = modelBookmarks->itemFromIndex(bookmarksList->currentIndex());
    if (item == NULL) { return; }
    QString path = item->data(MEDIA_PATH).toString();
    if (path.isEmpty()) { return; }
    disks->devices[path]->unmount();
}

void MainWindow::handleMediaEject()
{
    //qDebug() << "handle media eject";
    QStandardItem *item = modelBookmarks->itemFromIndex(bookmarksList->currentIndex());
    if (item == NULL) { return; }
    QString path = item->data(MEDIA_PATH).toString();
    if (path.isEmpty()) { return; }
    disks->devices[path]->eject();
}
#endif

void MainWindow::clearCache()
{
    settings->setValue("clearCache", true);
    QMessageBox::information(this, tr("Close window"), tr("Please close window to apply action."));
}

void MainWindow::handlePathRequested(QString path)
{
    qDebug() << "handle service path requested" << path;
    if (path == pathEdit->currentText() || path.isEmpty()) { return; }
    if (path.contains("/.")) { modelList->setRootPath(path); }
    tree->setCurrentIndex(modelTree->mapFromSource(modelList->index(path)));
    status->showMessage(getDriveInfo(curIndex.filePath()));
}
//---------------------------------------------------------------------------

void MainWindow::actionMapper(QString cmd)
{
    QModelIndexList selList;
    QStringList temp;

    if (focusWidget() == list || focusWidget() == detailTree) {
        QFileInfo file = modelList->fileInfo(modelView->mapToSource(listSelectionModel->currentIndex()));

        if (file.isDir()) {
            cmd.replace("%n",file.fileName().replace(" ","\\"));
        } else {
            cmd.replace("%n",file.baseName().replace(" ","\\"));
        }

        if (listSelectionModel->selectedRows(0).count()) { selList = listSelectionModel->selectedRows(0); }
        else { selList = listSelectionModel->selectedIndexes(); }
    }
    else {
        selList << modelView->mapFromSource(modelList->index(curIndex.filePath()));
    }

    cmd.replace("~",QDir::homePath());


    //process any input tokens
    int pos = 0;
    while(pos >= 0) {
        pos = cmd.indexOf("%i",pos);
        if(pos != -1) {
            pos += 2;
            QString var = cmd.mid(pos,cmd.indexOf(" ",pos) - pos);
            QString input = QInputDialog::getText(this,tr("Input"), var, QLineEdit::Normal);
            if(input.isNull()) { return; } // cancelled
            else { cmd.replace("%i" + var,input); }
        }
    }


    foreach(QModelIndex index,selList) {
        temp.append(modelList->fileName(modelView->mapToSource(index)).replace(" ","\\"));
    }

    cmd.replace("%f",temp.join(" "));

    temp.clear();

    foreach(QModelIndex index,selList) {
        temp.append(modelList->filePath(modelView->mapToSource(index)).replace(" ","\\"));
    }

    cmd.replace("%F",temp.join(" "));

    customActManager->execAction(cmd, pathEdit->itemText(0));
}

//---------------------------------------------------------------------------------
void MainWindow::clearCutItems()
{
    //this refreshes existing items, sizes etc but doesn't re-sort
    modelList->clearCutItems();
    modelList->update();

    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(pathEdit->currentText()));

    if (currentView == 2) { detailTree->setRootIndex(baseIndex); }
    else { list->setRootIndex(baseIndex); }
    QTimer::singleShot(50,this,SLOT(dirLoaded()));
    return;
}

//---------------------------------------------------------------------------------
bool mainTreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceModel() == NULL) { return false; }
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());
    if (fileModel == NULL) { return false; }
    if (fileModel->isDir(index0)) {
        if (this->filterRegExp().isEmpty() || fileModel->fileInfo(index0).isHidden() == 0) { return true; }
    }

    return false;
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (this->filterRegExp().isEmpty()) { return true; }

    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());

    if (fileModel->fileInfo(index0).isHidden()) { return false; }
    else { return true; }
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    myModel* fsModel = dynamic_cast<myModel*>(sourceModel());

    if ((fsModel->isDir(left) && !fsModel->isDir(right))) {
        return sortOrder() == Qt::AscendingOrder;
    } else if(!fsModel->isDir(left) && fsModel->isDir(right)) {
        return sortOrder() == Qt::DescendingOrder;
    }

    if(left.column() == 1) { // size
        if (fsModel->size(left) > fsModel->size(right)) { return true; }
        else { return false; }
    } else if (left.column() == 3) { // date
        if (fsModel->fileInfo(left).lastModified() > fsModel->fileInfo(right).lastModified()) { return true; }
        else { return false; }
    }

    return QSortFilterProxyModel::lessThan(left,right);
}

//---------------------------------------------------------------------------------
QStringList myCompleter::splitPath(const QString& path) const
{
    QStringList parts = path.split("/");
    parts[0] = "/";

    return parts;
}

//---------------------------------------------------------------------------------
QString myCompleter::pathFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) { return QString(); }

    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = model()->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

    list[0].clear() ; // the join below will provide the separator

    return list.join("/");
}
