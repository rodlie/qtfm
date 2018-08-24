#include "qtfm.h"
#include "common.h"
#include "fm.h"

#include <QMdiSubWindow>

QtFM::QtFM(QWidget *parent)
    : QMainWindow(parent)
    , mdi(nullptr)
    , mimes(nullptr)
    , mime(true)
{
    setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/fm/images/qtfm.png")));
    setWindowTitle("QtFM");

    mdi = new QMdiArea(this);
    setCentralWidget(mdi);

    mimes = new MimeUtils(this);
    mimes->setDefaultsFileName(Common::readSetting("defMimeAppsFile", MIME_APPS).toString());

    startPath = QDir::currentPath();
    QStringList args = qApp->arguments();
    if (args.count() > 1) {
        startPath = args.at(1);
        if (QUrl(startPath).isLocalFile()) {
            startPath = QUrl(args.at(1)).toLocalFile();
        }
    }

    QIcon::setThemeSearchPaths(Common::iconPaths(qApp->applicationDirPath()));
    Common::setupIconTheme(qApp->applicationFilePath());

    newSubWindow(startPath);
}

QtFM::~QtFM()
{

}

void QtFM::newSubWindow(QString path)
{
    if (path.isEmpty()) { return; }
    QMdiSubWindow *subwindow = new QMdiSubWindow;
    subwindow->setWidget(new FM(mime, mimes, path));
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(path);
    subwindow->setWindowIcon(windowIcon());
    mdi->addSubWindow(subwindow);
}
