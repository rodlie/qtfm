#include "qtfm.h"
#include "common.h"

#include <QMdiSubWindow>

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
    , backButton(Q_NULLPTR)
    , upButton(Q_NULLPTR)
    , homeButton(Q_NULLPTR)
    , tileButton(Q_NULLPTR)
{
    QIcon::setThemeSearchPaths(Common::iconPaths(qApp->applicationDirPath()));
    Common::setupIconTheme(qApp->applicationFilePath());

    setWindowIcon(QIcon::fromTheme("qtfm",
                                   QIcon(":/images/qtfm.png")));
    setWindowTitle("QtFM");

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
    //mdi->setTabPosition(QTabWidget::South);
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
    tileAction->setText(tr("Tile Tabs"));
    tileAction->setShortcut(QKeySequence(tr("Ctrl+S")));

    viewMenu->addAction(tileAction);

    mBar->addMenu(fileMenu);
    mBar->addMenu(editMenu);
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

    mimes = new MimeUtils(this);
    mimes->setDefaultsFileName(Common::readSetting("defMimeAppsFile",
                                                   MIME_APPS).toString());

    backButton->hide();
    upButton->hide();
    homeButton->hide();
    tileButton->hide();

    setupConnections();
    loadSettings();
    parseArgs();
}

QtFM::~QtFM()
{
    writeSettings();
}

void QtFM::newSubWindow(QString path)
{
    qDebug() << "newSubWindow" << path;
    QFileInfo info(path);
    if (!info.isDir()) { return; }

    QMdiSubWindow *subwindow = new QMdiSubWindow;
    FM *fm = new FM(mimes, path);

    connect(fm, SIGNAL(newWindowTitle(QString)),
            subwindow, SLOT(setWindowTitle(QString)));
    connect(fm, SIGNAL(updatedDir(QString)),
            this, SLOT(handleUpdatedDir(QString)));
    connect(fm, SIGNAL(newPath(QString)),
            this, SLOT(handleNewPath(QString)));

    subwindow->setWidget(fm);
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(info.completeBaseName());
    subwindow->setWindowIcon(windowIcon());
    mdi->addSubWindow(subwindow);//->setWindowState(Qt::WindowMaximized);
}

void QtFM::parseArgs()
{
    QStringList args = qApp->arguments();
    for (int i=1;i<args.count();++i) {
        if (!QFile::exists(args.at(i))) { continue; }
        newSubWindow(args.at(i));
    }
    if (mdi->subWindowList().count() == 0) {
        newSubWindow(QDir::currentPath());
    }
}

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
    /*
  connect(customComplete, SIGNAL(activated(QString)),
          this, SLOT(pathEditChanged(QString)));
  connect(pathEdit->lineEdit(), SIGNAL(cursorPositionChanged(int,int)),
          this, SLOT(addressChanged(int,int)));
*/
}

void QtFM::loadSettings()
{
    qDebug() << "load settings";
}

void QtFM::writeSettings()
{
    qDebug() << "write settings";
}

void QtFM::handleNewPath(QString path)
{
    Q_UNUSED(path)
    FM *fm = dynamic_cast<FM*>(sender());
    if (!mdi->currentSubWindow() || !fm) { return; }
    if (fm != dynamic_cast<FM*>(mdi->currentSubWindow()->widget())) { return; }
    qDebug() << "update path for tab";
    refreshPath(fm);
}

void QtFM::handleUpdatedDir(QString path)
{
    qDebug() << "handle updated dir" << path;
}

void QtFM::handleTabActivated(QMdiSubWindow *tab)
{
    if (!tab) { return; }
    FM *fm = dynamic_cast<FM*>(tab->widget());
    if (!fm) { return; }
    qDebug() << "handle tab activated" << fm->getPath();
    refreshPath(fm);
}

void QtFM::refreshPath(FM *fm)
{
    if (!fm) { return; }
    pathEdit->clear();
    pathEdit->setCompleter(fm->getCompleter());
    pathEdit->addItems(*fm->getHistory());
    pathEdit->setCurrentIndex(0);
}

void QtFM::pathEditChanged(const QString &path)
{
    qDebug() << "path edit changed" << path;

    QString info = path;
    if (!QFileInfo(path).exists()) {
        qDebug() << "path does not exists" << path;
        return;
    }

    info.replace(QString("~"), QDir::homePath());
    FM *fm = dynamic_cast<FM*>(mdi->currentSubWindow()->widget());
    if (!fm) { return; }

    qDebug() << "set new path in fm" << path;
    fm->setPath(path);
}
