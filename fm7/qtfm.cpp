#include "qtfm.h"
#include "common.h"

#include <QMdiSubWindow>
#include <QApplication>
#include <QFont>
#include <QLineEdit>

QtFM::QtFM(QWidget *parent)
    : QMainWindow(parent)
    , mdi(Q_NULLPTR)
    , mimes(Q_NULLPTR)
    , mBar(Q_NULLPTR)
    , sBar(Q_NULLPTR)
    , navBar(Q_NULLPTR)
    , pathEdit(Q_NULLPTR)
    , fileMenu(Q_NULLPTR)
    , editMenu(Q_NULLPTR)
    , viewMenu(Q_NULLPTR)
    , tileAction(Q_NULLPTR)
    , tabViewAction(Q_NULLPTR)
    , newTabAction(Q_NULLPTR)
    , newTermAction(Q_NULLPTR)
    , backButton(Q_NULLPTR)
    , upButton(Q_NULLPTR)
    , homeButton(Q_NULLPTR)
    , tileButton(Q_NULLPTR)
    , dockBookmarks(Q_NULLPTR)
    , bookmarksList(Q_NULLPTR)
    , modelBookmarks(Q_NULLPTR)
{
    // setup icon theme
    QIcon::setThemeSearchPaths(Common::iconPaths(qApp->applicationDirPath()));
    Common::setupIconTheme(qApp->applicationFilePath());

    // set window icon and title
    setWindowIcon(QIcon::fromTheme("folder",
                                   QIcon::fromTheme("qtfm",
                                                    QIcon(":/images/qtfm.png"))));
    setWindowTitle(tr("Qt File Manager"));

    // setup mime handler
    mimes = new MimeUtils(this);
    mimes->setDefaultsFileName(Common::readSetting("defMimeAppsFile",
                                                   MIME_APPS).toString());

    // setup bookmarks model
    modelBookmarks = new bookmarkmodel();

    // setup widgets
    setupWidgets();

    // setup connections
    setupConnections();

    // load settings
    loadSettings();

    // parse user arguments
    QTimer::singleShot(10, this, SLOT(parseArgs()));
}

QtFM::~QtFM()
{
    modelBookmarks->deleteLater();
    writeSettings();
}

void QtFM::newSubWindow(bool triggered)
{
    Q_UNUSED(triggered)
    newSubWindow();
}

// create a new file manager subwindow
void QtFM::newSubWindow(QString path)
{
    // we only allow directories
    QFileInfo info(path);
    if (!info.isDir()) { return; }

    // create a new subwindow and FM
    QMdiSubWindow *subwindow = new QMdiSubWindow(this);
    FM *fm = new FM(mimes, path);

    // connects
    connect(fm, SIGNAL(newWindowTitle(QString)),
            subwindow, SLOT(setWindowTitle(QString)));
    connect(fm, SIGNAL(updatedDir(QString)),
            this, SLOT(handleUpdatedDir(QString)));
    connect(fm, SIGNAL(newPath(QString)),
            this, SLOT(handleNewPath(QString)));
    connect(fm, SIGNAL(openFile(QString)),
            this, SLOT(handleOpenFile(QString)));
    connect(fm, SIGNAL(previewFile(QString)),
            this, SLOT(handlePreviewFile(QString)));

    // setup subwindow
    mdi->addSubWindow(subwindow);
    subwindow->setWidget(fm);
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(info.completeBaseName());
    subwindow->setWindowIcon(windowIcon());
    subwindow->setWindowState(Qt::WindowMaximized);

    refreshPath(fm); // connect pathEdit with FM

    // tile if we have more subwindows
    if (mdi->subWindowList().count()>1) { mdi->tileSubWindows(); }
}

// setup all the various widgets
void QtFM::setupWidgets()
{
    mBar = new QMenuBar(this);
    sBar = new QStatusBar(this);

    fileMenu = new QMenu(this);
    fileMenu->setTitle(tr("File"));

    editMenu = new QMenu(this);
    editMenu->setTitle(tr("Edit"));

    viewMenu = new QMenu(this);
    viewMenu->setTitle(tr("View"));

    navBar = new QToolBar(this);
    navBar->setWindowTitle(tr("Navigation"));

    pathEdit = new QComboBox(this);
    pathEdit->setEditable(true);
    pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mdi = new QMdiArea(this);
    mdi->setViewMode(QMdiArea::TabbedView);
    mdi->setTabPosition(QTabWidget::North);
    mdi->setTabsClosable(true);
    mdi->setTabsMovable(true);

    backButton = new QPushButton(this);
    backButton->setText(tr("Back"));

    upButton = new QPushButton(this);
    upButton->setText(tr("Up"));

    homeButton = new QPushButton(this);
    homeButton->setText(tr("Home"));

    tileButton = new QPushButton(this);
    tileButton->setText(tr("Tile"));

    tileAction = new QAction(this);
    tileAction->setText(tr("Tile"));
    tileAction->setIcon(QIcon::fromTheme("preferences-system-windows"));
    tileAction->setShortcut(QKeySequence(tr("Ctrl+T")));

    newTabAction = new QAction(this);
    newTabAction->setText(tr("Open new folder"));
    newTabAction->setIcon(QIcon::fromTheme("window-new"));
    newTabAction->setShortcut(QKeySequence(tr("Ctrl+N")));

    newTermAction = new QAction(this);
    newTermAction->setText(tr("Open new terminal"));
    newTermAction->setIcon(QIcon::fromTheme("terminal"));


    fileMenu->addAction(newTabAction);
    fileMenu->addAction(newTermAction);
    viewMenu->addAction(tileAction);

    mBar->addMenu(fileMenu);
    //mBar->addMenu(editMenu);
    mBar->addMenu(viewMenu);

    //navBar->addWidget(backButton);
    //navBar->addWidget(upButton);
    //navBar->addWidget(homeButton);
    navBar->addWidget(pathEdit);
    //navBar->addWidget(tileButton);

    setMenuBar(mBar);
    setStatusBar(sBar);
    addToolBar(Qt::TopToolBarArea, navBar);
    setCentralWidget(mdi);

    backButton->hide();
    upButton->hide();
    homeButton->hide();
    tileButton->hide();
editMenu->hide();


    dockBookmarks = new QDockWidget(tr("Bookmarks"), this, Qt::SubWindow);
    dockBookmarks->setObjectName("bookmarksDock");
    dockBookmarks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    dockBookmarks->setFeatures(QDockWidget::DockWidgetMovable);

    bookmarksList = new QListView(dockBookmarks);
    bookmarksList->setMinimumHeight(24);
    bookmarksList->setFocusPolicy(Qt::ClickFocus);
    bookmarksList->setDragDropMode(QAbstractItemView::DragDrop);
    bookmarksList->setDropIndicatorShown(true);
    bookmarksList->setDefaultDropAction(Qt::MoveAction);
    bookmarksList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    bookmarksList->setModel(modelBookmarks);
    bookmarksList->setResizeMode(QListView::Adjust);
    bookmarksList->setFlow(QListView::TopToBottom);
    bookmarksList->setIconSize(QSize(24, 24));

    dockBookmarks->setWidget(bookmarksList);


    addDockWidget(Qt::LeftDockWidgetArea, dockBookmarks);

}

// parse user arguments
void QtFM::parseArgs()
{
    QStringList args = qApp->arguments();
    if (args.count()<2) {
        newSubWindow(); // open a new subwindow
        return;
    }
    for (int i=1;i<args.count();++i) { // open multiple subwindows
        if (!QFile::exists(args.at(i))) { continue; } // ignore
        newSubWindow(args.at(i));
    }
}

// setup various connections
void QtFM::setupConnections()
{
    connect(tileButton, SIGNAL(released()),
            mdi, SLOT(tileSubWindows()));
    connect(tileAction, SIGNAL(triggered()),
            mdi, SLOT(tileSubWindows()));
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(handleTabActivated(QMdiSubWindow*)));

    connect(pathEdit, SIGNAL(activated(QString)),
            this, SLOT(pathEditChanged(QString)));

    connect(newTabAction, SIGNAL(triggered(bool)),
            this, SLOT(newSubWindow(bool)));
    connect(newTermAction, SIGNAL(triggered(bool)),
            this, SLOT(handleNewTermAction()));

    connect(modelBookmarks, SIGNAL(bookmarksChanged()),
            this, SLOT(handleBookmarksChanged()));
}

// load main app settings
void QtFM::loadSettings()
{
    qDebug() << "load settings";

    setupBookmarks();
}

// save main app settings
void QtFM::writeSettings()
{
    qDebug() << "write settings";

    writeBookmarks();
}

// update pathEdit from sender
void QtFM::handleNewPath(QString path)
{
    Q_UNUSED(path)
    FM *fm = dynamic_cast<FM*>(sender());
    if (!mdi->currentSubWindow() || !fm) { return; } // no subwindow(s)
    if (fm != dynamic_cast<FM*>(mdi->currentSubWindow()->widget())) { return; } // not sender
    qDebug() << "update path for tab";
    refreshPath(fm);
}

void QtFM::handleUpdatedDir(QString path)
{
    qDebug() << "handle updated dir" << path;
}

// stuff todo when a tab is activated
void QtFM::handleTabActivated(QMdiSubWindow *tab)
{
    if (!tab) { return; } // ignore

    // FM?
    FM *fm = dynamic_cast<FM*>(tab->widget());
    if (fm) { // get window title and path from fm
        qDebug() << "handle fm activated" << fm->getPath();
        tab->setWindowTitle(fm->getPath().split("/").takeLast());
        refreshPath(fm);
        return;
    }

    // TERM?
    QTermWidget *console = dynamic_cast<QTermWidget*>(tab->widget());
    if (console) { // get window title from term and clear pathedit
        qDebug() << "handle term activated" << console->workingDirectory();
        pathEdit->clear();
        pathEdit->setCurrentIndex(0);
        pathEdit->setCurrentText(console->workingDirectory());
        tab->setWindowTitle(console->workingDirectory().split("/").takeLast());
        return;
    }
}

void QtFM::handleOpenFile(const QString &file)
{
    qDebug() << "handle open file" << file;
}

void QtFM::handlePreviewFile(const QString &file)
{
    qDebug() << "handle preview file" << file;
}

// open new terminal from action
void QtFM::handleNewTermAction(const QString &path)
{
    newTerminal(path.isEmpty()?QDir::homePath():path);
}

// handle terminal title changed
void QtFM::handleTermTitleChanged()
{
    QTermWidget *console = dynamic_cast<QTermWidget*>(sender());
    if (!mdi->currentSubWindow() || !console) { return; }
    if (console != dynamic_cast<QTermWidget*>(mdi->currentSubWindow()->widget())) { return; }
    mdi->currentSubWindow()->setWindowTitle(console->workingDirectory().split("/").takeLast());
}

// update path from FM
void QtFM::refreshPath(FM *fm)
{
    if (!fm) { return; } // ignore
    pathEdit->clear();
    pathEdit->setCompleter(fm->getCompleter()); // get completer
    pathEdit->addItems(*fm->getHistory()); // get history
    pathEdit->setCurrentIndex(0);
}

// handle path changed
void QtFM::pathEditChanged(const QString &path)
{
    qDebug() << "path edit changed" << path;
    if (!mdi->currentSubWindow()) { return; } // nothing to do

    // FM
    FM *fm = dynamic_cast<FM*>(mdi->currentSubWindow()->widget());
    if (fm) {
        QString info = path;
        if (!QFileInfo(path).exists()) { return; } // ignore
        info.replace(QString("~"), QDir::homePath());
        qDebug() << "set new fm path in tab" << path;
        fm->setPath(path);
        return;
    }

    // TERM
    /*QTermWidget *console = dynamic_cast<QTermWidget*>(mdi->currentSubWindow()->widget());
    if (console) {
        qDebug() << "change working directory in terminal" << console->workingDirectory() << path;
        if (console->workingDirectory() != path) {
            console->setWorkingDirectory(path);
            //console->update();
            //console->clear();
        }
        return;
    }*/
}

// EXPERIMENTAL!
void QtFM::newTerminal(const QString &path)
{
    QFileInfo info(path);
    if (!info.isDir()) { return; }

    qDebug() << "new terminal" << path;
    QMdiSubWindow *subwindow = new QMdiSubWindow(this);
    QTermWidget *console = new QTermWidget(this);

    QFont font = QApplication::font();
#ifdef Q_OS_MACOS
    font.setFamily("Monaco");
#elif defined(Q_WS_QWS)
    font.setFamily("fixed");
#else
    font.setFamily("Monospace");
#endif
    font.setPointSize(9);
    console->setTerminalFont(font);
    console->setTerminalSizeHint(true);
    //console->setBidiEnabled(true);
    //console->setBlinkingCursor(true);
    if (console->availableColorSchemes().contains("WhiteOnBlack")) {
        console->setColorScheme("WhiteOnBlack");
    }
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);
    console->setWorkingDirectory(path);

    connect(console, SIGNAL(finished()), subwindow, SLOT(close()));
    connect(console, SIGNAL(titleChanged()), this, SLOT(handleTermTitleChanged()));

    mdi->addSubWindow(subwindow);
    subwindow->setWidget(console);
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(console->workingDirectory());
    subwindow->setWindowIcon(QIcon::fromTheme("utilities-terminal"));
    subwindow->setWindowState(Qt::WindowMaximized);

    if (mdi->subWindowList().count()>1) {
        mdi->tileSubWindows();
    }
}

// setup bookmarks
void QtFM::setupBookmarks()
{


    // Remove old
    modelBookmarks->removeRows(0, modelBookmarks->rowCount());

    // Load
    //loadBookmarks();

    // first-run (if no config)
    modelBookmarks->addBookmark(tr("Computer"), "/", "", "computer", "", false, false);
#ifdef Q_OS_MAC
    modelBookmarks->addBookmark(tr("Applications"), "/Applications", "", "applications-other", "", false, false);
#endif
    modelBookmarks->addBookmark(tr("Home"), QDir::homePath(), "", "user-home", "", false, false);
    if (QFile::exists(QString("%1/Desktop").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Desktop"), QString("%1/Desktop").arg(QDir::homePath()), "", "user-desktop", "", false, false);
    }
    if (QFile::exists(QString("%1/Documents").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Documents"), QString("%1/Documents").arg(QDir::homePath()), "", "text-x-generic", "", false, false);
    }
    if (QFile::exists(QString("%1/Dowloads").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Downloads"), QString("%1/Dowloads").arg(QDir::homePath()), "", "applications-internet", "", false, false);
    }
    if (QFile::exists(QString("%1/Pictures").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Pictures"), QString("%1/Pictures").arg(QDir::homePath()), "", "image-x-generic", "", false, false);
    }
    if (QFile::exists(QString("%1/Videos").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Videos"), QString("%1/Videos").arg(QDir::homePath()), "", "video-x-generic", "", false, false);
    }
    if (QFile::exists(QString("%1/Music").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Music"), QString("%1/Music").arg(QDir::homePath()), "", "audio-x-generic", "", false, false);
    }
    if (QFile::exists(QString("%1/.local/share/Trash").arg(QDir::homePath()))) {
        modelBookmarks->addBookmark(tr("Trash"), QString("%1/.local/share/Trash").arg(QDir::homePath()), "", "user-trash", "", false, false);
    }
    //writeBookmarks();
}

void QtFM::writeBookmarks()
{
    qDebug() << "write bookmarks";
}

void QtFM::handleBookmarksChanged()
{
    writeBookmarks();
}
