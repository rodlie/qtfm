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
#include <QMenuBar>
#ifndef NO_DBUS
#include <QDBusConnection>
#include <QDBusError>
#endif
#include <fcntl.h>

#include <QtConcurrent/QtConcurrent>

#include "mainwindow.h"
#include "mymodel.h"
#include "fileutils.h"
#include "applicationdialog.h"

#include "common.h"

#include "qtcopydialog.h"
#include "qtfilecopier.h"

#ifdef Q_OS_MAC
#include <QStyleFactory>
#endif

MainWindow::MainWindow()
{
    // setup icon theme search path
    QStringList iconsPath = QIcon::themeSearchPaths();
    QString iconsHomeLocal = QString("%1/.local/share/icons").arg(QDir::homePath());
    QString iconsHome = QString("%1/.icons").arg(QDir::homePath());
    if (QFile::exists(iconsHomeLocal) && !iconsPath.contains(iconsHomeLocal)) { iconsPath.prepend(iconsHomeLocal); }
    if (QFile::exists(iconsHome) && !iconsPath.contains(iconsHome)) { iconsPath.prepend(iconsHome); }
    iconsPath << QString("%1/../share/icons").arg(qApp->applicationDirPath());
    QIcon::setThemeSearchPaths(iconsPath);
    qDebug() << "using icon theme search path" << QIcon::themeSearchPaths();

    // libdisks
#ifndef NO_UDISKS
    disks = new Disks(this);
    connect(disks, SIGNAL(updatedDevices()), this, SLOT(populateMedia()));
    connect(disks, SIGNAL(mountpointChanged(QString,QString)), this, SLOT(handleMediaMountpointChanged(QString,QString)));
    connect(disks, SIGNAL(foundNewDevice(QString)), this, SLOT(handleMediaAdded(QString)));
    connect(disks, SIGNAL(removedDevice(QString)), this, SLOT(handleMediaRemoved(QString)));
    connect(disks, SIGNAL(mediaChanged(QString,bool)), this, SLOT(handleMediaChanged(QString,bool)));
    connect(disks, SIGNAL(deviceErrorMessage(QString,QString)), this, SLOT(handleMediaError(QString,QString)));
#endif

    // dbus service
#ifndef NO_DBUS
    if (QDBusConnection::sessionBus().isConnected()) {
        if (QDBusConnection::sessionBus().registerService(FM_SERVICE)) {
            service = new qtfm(this);
            connect(service, SIGNAL(pathRequested(QString)), this, SLOT(handlePathRequested(QString)));
            if (!QDBusConnection::sessionBus().registerObject(FM_PATH, service, QDBusConnection::ExportAllSlots)) {
                qWarning() << QDBusConnection::sessionBus().lastError().message();
            }
        } else { qWarning() << QDBusConnection::sessionBus().lastError().message(); }
    }
#endif

    // get path from cmd
    startPath = QDir::currentPath();
    QStringList args = QApplication::arguments();

    if(args.count() > 1) {
        startPath = args.at(1);
        if (startPath == ".") {
            startPath = getenv("PWD");
        } else if (QUrl(startPath).isLocalFile()) {
            startPath = QUrl(args.at(1)).toLocalFile();
        }
    }

    settings = new QSettings(Common::configFile(), QSettings::IniFormat);
    if (settings->value("clearCache").toBool()) {
        qDebug() << "clear cache";
        Common::removeFileCache();
        Common::removeFolderCache();
        Common::removeThumbsCache();
        settings->setValue("clearCache", false);
    }

    // Dark theme?
    if (settings->value("darkTheme").toBool()) {
        qApp->setPalette(Common::darkTheme());
    }

    // set icon theme
    Common::setupIconTheme(qApp->applicationFilePath());

    // Create mime utils
    mimeUtils = new MimeUtils(this);
    QString name = settings->value("defMimeAppsFile", MIME_APPS).toString();
    mimeUtils->setDefaultsFileName(name);

    // Create filesystem model
    bool realMime = settings->value("realMimeTypes", true).toBool();
    modelList = new myModel(realMime, mimeUtils, this);
    connect(modelList, SIGNAL(reloadDir(QString)), this, SLOT(handleReloadDir(QString)));

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
    bookmarksList->setFocusPolicy(Qt::ClickFocus); // Avoid hijacking focus when Tab on Edit Path
    dockBookmarks->setWidget(bookmarksList);
    addDockWidget(Qt::LeftDockWidgetArea, dockBookmarks);

    QWidget *main = new QWidget(this);
    mainLayout = new QVBoxLayout(main);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    stackWidget = new QStackedWidget(this);
    QWidget *page = new QWidget(this);
    QHBoxLayout *hl1 = new QHBoxLayout(page);
    hl1->setSpacing(0);
    hl1->setContentsMargins(0,0,0,0);
    list = new QListView(page);
    hl1->addWidget(list);
    stackWidget->addWidget(page);

    QWidget *page2 = new QWidget(this);
    hl1 = new QHBoxLayout(page2);
    hl1->setSpacing(0);
    hl1->setContentsMargins(0,0,0,0);
    detailTree = new DfmQTreeView(page2);
    hl1->addWidget(detailTree);
    stackWidget->addWidget(page2);

    tabs = new tabBar(modelList->folderIcons);

    mainLayout->addWidget(stackWidget);
    mainLayout->addWidget(tabs);

    setCentralWidget(main);

    modelTree = new viewsSortProxyModel();
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

    detailTree->setRootIsDecorated(true);
    detailTree->setItemsExpandable(true);
    detailTree->setUniformRowHeights(true);
    detailTree->setAlternatingRowColors(true);
    detailTree->setModel(modelView);
    detailTree->setSelectionModel(listSelectionModel);

    pathEdit = new QComboBox(this);
    pathEdit->setEditable(true);
    pathEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    pathEdit->setMinimumWidth(100);

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

    createActions();
    createToolBars();
    createMenus();

    setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/images/qtfm.png")));
    setWindowTitle(APP_NAME);

    // Create custom action manager
    customActManager = new CustomActionsManager(settings, actionList, this);

    // Create bookmarks model
    modelBookmarks = new bookmarkmodel(/*modelList->folderIcons*/);
    connect(modelBookmarks, SIGNAL(bookmarksChanged()), this, SLOT(handleBookmarksChanged()));

    // Load settings before showing window
    loadSettings();

    // show window
    show();

    trashDir = Common::trashDir();
    ignoreReload = false;

    qApp->installEventFilter(this);

    QTimer::singleShot(0, this, SLOT(lateStart()));
}
//---------------------------------------------------------------------------

/**
 * @brief Initialization
 */
void MainWindow::lateStart() {

  // Update status panel
  status->showMessage(Common::getDriveInfo(curIndex.filePath()));

  // Configure bookmarks list
  bookmarksList->setDragDropMode(QAbstractItemView::DragDrop);
  bookmarksList->setDropIndicatorShown(true);
  bookmarksList->setDefaultDropAction(Qt::MoveAction);
  bookmarksList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Configure tree view
  tree->setDragDropMode(QAbstractItemView::DragDrop);
  tree->setDefaultDropAction(Qt::MoveAction);
  tree->setDropIndicatorShown(true);
  tree->setEditTriggers(QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::SelectedClicked);

  // Configure detail view
  detailTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
  detailTree->setDragDropMode(QAbstractItemView::DragDrop);
  detailTree->setDefaultDropAction(Qt::MoveAction);
  detailTree->setDropIndicatorShown(true);
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
  clipboardChanged();

  // Completer configuration
  customComplete = new myCompleter();
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

  // Connect bookmark model
  connect(modelBookmarks,
          SIGNAL(bookmarkPaste(const QMimeData *, QString, QStringList, bool)), this,
          SLOT(pasteLauncher(const QMimeData *, QString, QStringList, bool)));
  connect(modelBookmarks, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(readShortcuts()));
  connect(modelBookmarks, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          this, SLOT(readShortcuts()));

  // Connect list model
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

  connect(modelList, SIGNAL(thumbUpdate(QString)),
          this, SLOT(thumbUpdate(QString)));

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

  // fix style
  setStyleSheet("QToolBar { padding: 0;border:none; }"
                /*"QFrame { border:none; }"
                "QListView::item,QListView::text,QListView::icon"
                "{ border:0px;padding-top:5px;padding-left:5px; }"*/);
  addressToolBar->setContentsMargins(0,0,5,0);

  // Restore window state
  if (wState) {
      qDebug() << "restore window state";
      if (!settings->value("windowState").isValid()) { // don't show dock tree/app as default
          dockTree->hide();
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
  zoomBook = settings->value("zoomBook", 24).toInt();
  zoomList = settings->value("zoomList", 24).toInt();
  zoomDetail = settings->value("zoomDetail", 32).toInt();
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
#ifndef Q_OS_MAC
  firstRunCustomActions(isFirstRun);
#endif

  // Load information whether tabs can be shown on top
  if (tabState) {
      tabsOnTopAct->setChecked(settings->value("tabsOnTop", 0).toBool());
      tabsOnTop();
  }

  // show/hide buttons
  homeAct->setVisible(settings->value("home_button", true).toBool());
  newTabAct->setVisible(settings->value("newtab_button", false).toBool());
  terminalAct->setVisible(settings->value("terminal_button", true).toBool());

  // path history
  pathHistory = settings->value("pathHistory", true).toBool();

  // path in window title
  showPathInWindowTitle = settings->value("windowTitlePath", true).toBool();
  if (!showPathInWindowTitle) { setWindowTitle(APP_NAME); }
  else {
    QFileInfo name = modelList->fileInfo(modelList->index(startPath));
    if (name.exists() && !name.isFile()) {
        if (name.fileName().isEmpty()) { setWindowTitle(name.absolutePath()); }
        else { setWindowTitle(curIndex.fileName()); }
    }
  }

  // 'copy of' filename
  copyXof = settings->value("copyXof", COPY_X_OF).toString();
  copyXofTS = settings->value("copyXofTS", COPY_X_TS).toString();
}

void MainWindow::firstRunBookmarks(bool isFirstRun)
{
    if (!isFirstRun) { return; }
    //qDebug() << "first run, setup default bookmarks";
    modelBookmarks->addBookmark(tr("Computer"), "/", "", "computer", "", false, false);
#ifdef Q_OS_MAC
    modelBookmarks->addBookmark(tr("Applications"), "/Applications", "", "applications-other", "", false, false);
#endif
    modelBookmarks->addBookmark(tr("Home"), QDir::homePath(), "", "user-home", "", false, false);
    modelBookmarks->addBookmark(tr("Desktop"), QString("%1/Desktop").arg(QDir::homePath()), "", "user-desktop", "", false, false);
    //modelBookmarks->addBookmark(tr("Documents"), QString("%1/Documents").arg(QDir::homePath()), "", "text-x-generic", "", false, false);
    //modelBookmarks->addBookmark(tr("Downloads"), QString("%1/Downloads").arg(QDir::homePath()), "", "applications-internet", "", false, false);
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
      QString number = QString("%1").arg(i, 4, 10, QChar('0'));
      settings->setValue(number, temp);
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

    QVector<QStringList> defActions = Common::getDefaultActions();
    for (int i=0;i<defActions.size();++i) {
        settings->setValue(QString(i), defActions.at(i));
    }

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
    qDebug() << "treeSelectionChanged";
    Q_UNUSED(previous)

    QFileInfo name = modelList->fileInfo(modelTree->mapToSource(current));
    if (!name.exists() || name.isFile()) { return; }

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
    updateGrid();
    qDebug() << "trigger dirloaded on tree selection changed";
    QTimer::singleShot(30,this,SLOT(dirLoaded()));
}

//---------------------------------------------------------------------------
void MainWindow::dirLoaded(bool thumbs)
{

    if (backIndex.isValid()) {
        backIndex = QModelIndex();
        return;
    }

    qDebug() << "dirLoaded triggered, thumbs?" << thumbs;
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
    else { total = Common::formatSize(bytes); }

    statusName->clear();
    statusSize->setText(QString("%1 items").arg(items.count()));
    statusDate->setText(QString("%1").arg(total));

    if (thumbsAct->isChecked() && thumbs) { QtConcurrent::run(modelList,&myModel::loadThumbs,items); }
    updateGrid();
}

void MainWindow::updateDir()
{
    dirLoaded(false /* don't refresh thumb*/);
}

void MainWindow::handleReloadDir(const QString &path)
{
    if (ignoreReload) {
        qDebug() << "ignore reload";
        return;
    }
    ignoreReload = true;
    qDebug() << "handle reload dir" << path << modelList->getRootPath();
    if (path != modelList->getRootPath()) { return; }
    dirLoaded();
    QTimer::singleShot(500, this, SLOT(enableReload()));
}

void MainWindow::thumbUpdate(const QString &path)
{
    qDebug() << "thumbupdate" << path << modelList->getRootPath();
    if (path != modelList->getRootPath()) { return; }
    refresh(false, false);
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
    else { total = Common::formatSize(bytes); }

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
    } else {
        addTab(QDir::homePath());
    }
}

void MainWindow::openNewTab()
{
    QFileInfo info(curIndex.filePath());
    if (!info.isDir()) { return; }
    addTab(curIndex.filePath());
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
#ifdef Q_OS_MAC
  if (modelList->isDir(modelView->mapToSource(current)) && !modelList->fileName(modelView->mapToSource(current)).endsWith(".app")) {
#else
  if (modelList->isDir(modelView->mapToSource(current))) {
#endif
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
 * @brief Handle clipboard changes
 */
void MainWindow::clipboardChanged()
{
    qDebug() << "clipboard changed";
    if (QApplication::clipboard()->mimeData()) {
        if (QApplication::clipboard()->mimeData()->hasUrls()) {
            qDebug() << "clipboard has data, enable paste";
            pasteAct->setEnabled(true);
            return;
        }
    }
    // clear tmp and disable paste if no mime
    modelList->clearCutItems();
    pasteAct->setEnabled(false);
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
    box.setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/images/qtfm.png")));
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
    currentDragMode = dragMode;
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
void MainWindow::pasteLauncher(const QMimeData *data,
                               const QString &newPath,
                               const QStringList &cutList,
                               bool link)
{
  QList<QUrl> files = data->urls();
  if (files.isEmpty()) { return; }
  pasteLauncher(files, newPath, cutList, link);
}
//---------------------------------------------------------------------------


void MainWindow::pasteLauncher(const QList<QUrl> &files,
                                const QString &newPath,
                                const QStringList &cutList,
                                bool link)
{
    //qDebug() << "==> PASTE LAUNCHER v2" << "COPY" << files << "OR MOVE" << cutList << "TO" << newPath << "SYMLINK?" << link;

    if (!QFile::exists(newPath)) {
        qDebug() << "destination path does not exists" << newPath;
        return;
    }
    if (files.size()==0 && cutList.size()==0) {
        qDebug() << "nothing to copy or move ...";
        return;
    }
    if (link && (files.size()==0 || cutList.size()>0)) {
        qDebug() << "is symlink but nothing to copy ...";
        return;
    }

    QStringList _files, _dirs;
    if (cutList.size()>0) { // move
        for (int i=0; i<cutList.size(); i++) {
            QFileInfo info(cutList.at(i));
            if (!info.isDir() && !info.isFile()) { continue; }
            if (cutList.at(i) == (newPath + "/" + info.fileName())) { continue; }
            if (info.isDir()) { _dirs << info.absoluteFilePath(); }
            else if (info.isFile()) { _files << info.absoluteFilePath(); }
        }
        if (_files.size()>0 || _dirs.size()>0) {
            QtFileCopier *copyHandler = new QtFileCopier(this);
            QtCopyDialog *copyDialog = new QtCopyDialog(copyHandler, this);
            copyDialog->setMinimumDuration(100);
            copyDialog->setAutoClose(true);
            if (_files.size()>0) {
                copyHandler->moveFiles(_files, newPath);
                //qDebug() << "MOVE FILES" << _files << "TO" << newPath;
            }
            if (_dirs.size()>0) {
                for (int i=0;i<_dirs.size();++i) {
                    copyHandler->moveDirectory(_dirs.at(i), newPath);
                    //qDebug() << "MOVE DIR" << _dirs.at(i) << "TO" << newPath;
                }
            }
        }
    } else if (files.size()>0) { // copy
        for (int i=0; i<files.size(); i++) {
            QFileInfo info(files.at(i).toLocalFile());
            if (!info.isDir() && !info.isFile()) { continue; }
            if (info.isDir()) { _dirs << info.absoluteFilePath(); }
            else if (info.isFile()) { _files << info.absoluteFilePath(); }
        }
        if (_files.size()>0 || _dirs.size()>0) {
            QtFileCopier *copyHandler = new QtFileCopier(this);
            QtCopyDialog *copyDialog = new QtCopyDialog(copyHandler, this);
            copyDialog->setMinimumDuration(100);
            copyDialog->setAutoClose(true);
            if (_files.size()>0) {
                if (link) { copyHandler->copyFiles(_files, newPath, QtFileCopier::MakeLinks); }
                else { copyHandler->copyFiles(_files, newPath); }
                //qDebug() << "COPY FILES" << _files << "TO" << newPath << "SYMLINK?" << link;
            }
            if (_dirs.size()>0) {
                for (int i=0;i<_dirs.size();++i) {
                    if (link) { copyHandler->copyDirectory(_dirs.at(i), newPath, QtFileCopier::MakeLinks); }
                    else { copyHandler->copyDirectory(_dirs.at(i), newPath); }
                    //qDebug() << "COPY DIR" <<_dirs.at(i) << "TO" << newPath << "SYMLINK?" << link;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------



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

  // Retrieve widget under mouse
  QMenu *popup;
  QWidget *widget = childAt(event->pos());
  //qDebug() << "WIDGET" << widget;

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
  } else if (widget == navToolBar) {
      qDebug() << "TOOLBAR";
      return;
  }

  QToolButton *isToolButton = dynamic_cast<QToolButton*>(childAt(event->pos()));
  if (isToolButton) {
      qDebug() << "TOOLBUTTON";
      return;
  }

#ifndef Q_OS_MAC
  QMenuBar *isMenuBar = dynamic_cast<QMenuBar*>(childAt(event->pos()));
  if (isMenuBar) {
      qDebug() << "MENUBAR";
      return;
  }
#endif

  // Continue with popups for folders and files
  QList<QAction*> actions;
  popup = new QMenu(this);

  bool isMedia = false;
  bool isTreeFile = false;

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

        // Add custom actions to the list of actions
        QHashIterator<QString, QAction*> i(*customActManager->getActions());
        while (i.hasNext()) {
          i.next();
          qDebug() << "custom action" << i.key() << i.key() << i.value();
          if (curIndex.completeSuffix().endsWith(i.key())) { actions.append(i.value()); }
        }

        // Add run action or open with default application action
        if (curIndex.isExecutable() || curIndex.isBundle() || type.endsWith("appimage") || curIndex.absoluteFilePath().endsWith(".desktop")) {
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
#ifndef Q_OS_MAC
        popup->addSeparator();
        popup->addMenu(createOpenWithMenu());
#endif
        //if (popup->actions().count() == 0) popup->addAction(openAct);

        // Add custom actions that are associated only with this file type
        if (!actions.isEmpty()) {
          popup->addSeparator();
          popup->addActions(actions);
          popup->addSeparator();
        }

        // Add menus
        // TODO: ???
        QHashIterator<QString, QMenu*> m(*customActManager->getMenus());
        while (m.hasNext()) {
          m.next();
          if (curIndex.completeSuffix().endsWith(m.key())) { popup->addMenu(m.value()); }
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
        //popup->addAction(openAct);
        popup->addAction(openInTabAct);
        popup->addSeparator();
        popup->addAction(addBookmarkAct);
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
      popup->addAction(backAct);
      popup->addAction(upAct);
      popup->addAction(homeAct);
      popup->addSeparator();
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
            if (!curIndex.path().isEmpty()) {
                popup->addAction(editBookmarkAct);	//icon
            }
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
        popup->addAction(addSeparatorAct);	//separator
        popup->addAction(wrapBookmarksAct);
      }
      popup->addSeparator();
    } else {
      // tree
      curIndex = modelList->filePath(modelTree->mapToSource(tree->currentIndex()));
      if (curIndex.isFile()) { isTreeFile = true;}

      bookmarksList->clearSelection();
      if (!isTreeFile) { // only for folders (for now)
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
    }
    popup->addSeparator();

    if (!isTreeFile) { // not a selected file in tree (dock)
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
        if (!isMedia && !curIndex.path().isEmpty()) { popup->addAction(folderPropertiesAct); }
    }
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

  qDebug() << "open with";
  // Add open with functionality ...
  QMenu *openMenu = new QMenu(tr("Open with"));

  // Select action
  QAction *selectAppAct = new QAction(tr("Select..."), openMenu);
  selectAppAct->setStatusTip(tr("Select application for opening the file"));
  //selectAppAct->setIcon(actionIcons->at(18));
  connect(selectAppAct, SIGNAL(triggered()), this, SLOT(selectAppForFiles()));

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

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        switch (me->button()) {
        case Qt::BackButton:
            goBackDir();
            break;
        default:;
        }
    }

      if (dynamic_cast<QListView*>(o) != Q_NULLPTR ){
        if (e->type()==QEvent::KeyPress) {
            QKeyEvent* key = static_cast<QKeyEvent*>(e);
            if ( (key->key()==Qt::Key_Tab) ) {
                qDebug()<< "Tab pressed: path completion "<< o ;
                QListView *completionList = dynamic_cast<QListView*>(o);
                // Remove incomplete phrase and replace it with a current index
                QModelIndex index = completionList->currentIndex();
                QString itemText = index.data(Qt::DisplayRole).toString();
                QString currentPath = pathEdit->lineEdit()->text();
                QStringList tempList = currentPath.split("/");
                tempList.takeLast();
                tempList << itemText;
                QString newPath = tempList.join("/");
                pathEdit->lineEdit()->setText(newPath);
                // Force update the Main View
                pathEditChanged(newPath);
                // Enter Edit Mode right after the event system is ready
                QTimer::singleShot(0, pathEdit->lineEdit(), SLOT(setFocus()));
                // Add the trailing / for subsequent completions
                pathEdit->lineEdit()->setText(newPath + QString("/"));
            }
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::refresh(bool modelRefresh, bool loadDir)
{
    qDebug() << "refresh" << modelRefresh << loadDir;
    if (modelRefresh) {
        modelList->refreshItems();
        modelList->forceRefresh();
    }

    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(pathEdit->currentText()));
    if (currentView == 2) { detailTree->setRootIndex(baseIndex); }
    else { list->setRootIndex(baseIndex); }

    pathEditChanged(pathEdit->currentText());

    if (loadDir) {
        qDebug() << "trigger dirloaded from refresh";
        dirLoaded();
    }
}

void MainWindow::enableReload()
{
    qDebug() << "enable reload";
    ignoreReload = false;
}
//---------------------------------------------------------------------------

/**
 * @brief Selects application for opening file
 */
void MainWindow::selectApp() {
  // Select application in the dialog
  ApplicationDialog *dialog = new ApplicationDialog(true, this);
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

void MainWindow::selectAppForFiles()
{
    // Selection
    QModelIndexList items;
    if (listSelectionModel->selectedRows(0).count()) {
      items = listSelectionModel->selectedRows(0);
    } else {
      items = listSelectionModel->selectedIndexes();
    }

    // Files
    QStringList files;
    foreach (QModelIndex index, items) {
      //executeFile(index, 0);
      QModelIndex srcIndex = modelView->mapToSource(index);
      files << modelList->filePath(srcIndex);
    }

    // Select application in the dialog
    ApplicationDialog *dialog = new ApplicationDialog(true, this);
    if (dialog->exec()) {
      if (dialog->getCurrentLauncher().compare("") != 0) {
        QString appName = dialog->getCurrentLauncher() + ".desktop";
        QString desktop = Common::findApplication(qApp->applicationFilePath(), appName);
        if (desktop.isEmpty()) { return; }
        DesktopFile df = DesktopFile(desktop);
        if (df.getExec().contains("%F") || df.getExec().contains("%U")) { // app supports multiple files
            mimeUtils->openFilesInApp(df.getExec(), files, df.isTerminal()?term:"");
        } else { // launch new instance for each file
            for (int i=0;i<files.size();++i) {
                QFileInfo fileInfo(files.at(i));
                mimeUtils->openInApp(df.getExec(), fileInfo, df.isTerminal()?term:"");
            }
        }
      }
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Opens files in application
 */
void MainWindow::openInApp()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    if (!action) { return; }
    DesktopFile df = DesktopFile(action->data().toString());
    if (df.getExec().isEmpty()) { return; }

    // get selection
    QModelIndexList items;
    if (listSelectionModel->selectedRows(0).count()) {
        items = listSelectionModel->selectedRows(0);
    } else {
        items = listSelectionModel->selectedIndexes();
    }

    // get files and mimes
    QStringList fileList;
    foreach (QModelIndex index, items) {
        QModelIndex srcIndex = modelView->mapToSource(index);
        QString filePath = modelList->filePath(srcIndex);
        fileList << filePath;
    }

    if (df.getExec().contains("%F") || df.getExec().contains("%U")) { // app supports multiple files
        mimeUtils->openFilesInApp(df.getExec(), fileList, df.isTerminal()?term:"");
    } else { // launch new instance for each file
        for (int i=0;i<fileList.size();++i) {
            QFileInfo fileInfo(fileList.at(i));
            mimeUtils->openInApp(df.getExec(), fileInfo, df.isTerminal()?term:"");
        }
    }
}

void MainWindow::updateGrid()
{
    if (list->viewMode() != QListView::IconMode) { return; }
    qDebug() << "updateGrid";
    QFontMetrics fm = fontMetrics();
    int textWidth = fm.averageCharWidth() * 17;
    int realTextWidth = fm.averageCharWidth() * 14;
    int textHeight = fm.lineSpacing() * 3;
    QSize grid;
    grid.setWidth(qMax(zoom, textWidth));
    grid.setHeight(zoom+textHeight);

    QModelIndexList items;
    for (int x = 0; x < modelList->rowCount(modelList->index(pathEdit->currentText())); ++x) {
        items.append(modelList->index(x,0,modelList->index(pathEdit->currentText())));
    }
    foreach (QModelIndex theItem,items) {
        QString filename = modelList->fileName(theItem);
        QRect item(0,0,realTextWidth,grid.height());
        QSize txtsize = fm.boundingRect(item, Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap|Qt::TextWrapAnywhere, filename).size();
        int newHeight = txtsize.height()+zoom+5+8+4;
        if  (txtsize.width()>grid.width()) { grid.setWidth(txtsize.width()); }
        if (newHeight>grid.height()) { grid.setHeight(newHeight); }
    }
    if (list->gridSize() != grid) {
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
#ifndef __FreeBSD__
                || (!device.value()->isOptical && !device.value()->isRemovable)
#endif
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
    if (item == Q_NULLPTR) { return; }
    QString path = item->data(MEDIA_PATH).toString();
    if (path.isEmpty()) { return; }
    disks->devices[path]->unmount();
}

void MainWindow::handleMediaEject()
{
    //qDebug() << "handle media eject";
    QStandardItem *item = modelBookmarks->itemFromIndex(bookmarksList->currentIndex());
    if (item == Q_NULLPTR) { return; }
    QString path = item->data(MEDIA_PATH).toString();
    if (path.isEmpty()) { return; }
    disks->devices[path]->eject();
}

void MainWindow::handleMediaError(QString path, QString error)
{
    QMessageBox::warning(this, path, error);
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
    pathEdit->setItemText(0, path);
    QTimer::singleShot(100, this, SLOT(slowPathEdit()));
}

void MainWindow::slowPathEdit()
{
    pathEditChanged(pathEdit->currentText());
    status->showMessage(Common::getDriveInfo(curIndex.filePath()));
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
    qDebug() << "clearCutItems";
    //this refreshes existing items, sizes etc but doesn't re-sort
    modelList->clearCutItems();
    modelList->update();

    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(pathEdit->currentText()));

    if (currentView == 2) { detailTree->setRootIndex(baseIndex); }
    else { list->setRootIndex(baseIndex); }

    qDebug() << "trigger updateDir from clearCutItems";
    QTimer::singleShot(50,this,SLOT(updateDir()));
}
