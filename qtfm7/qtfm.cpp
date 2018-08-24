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
    QIcon::setThemeSearchPaths(Common::iconPaths(qApp->applicationDirPath()));
    Common::setupIconTheme(qApp->applicationFilePath());

    setWindowIcon(QIcon::fromTheme("qtfm", QIcon(":/fm/images/qtfm.png")));
    setWindowTitle("QtFM");

    mdi = new QMdiArea(this);
    setCentralWidget(mdi);

    mimes = new MimeUtils(this);
    mimes->setDefaultsFileName(Common::readSetting("defMimeAppsFile", MIME_APPS).toString());

    QStringList args = qApp->arguments();
    if (args.count() > 1) {
        for (int i=1;i<args.count();++i) {
            if (!QFile::exists(args.at(i))) { continue; }
            newSubWindow(args.at(i));
        }
    }
    if (mdi->subWindowList().count() == 0) {
        newSubWindow(QDir::currentPath());
    }
}

QtFM::~QtFM()
{

}

void QtFM::newSubWindow(QString path)
{
    qDebug() << "newSubWindow" << path;
    QFileInfo info(path);
    if (!info.isDir()) { return; }
    QMdiSubWindow *subwindow = new QMdiSubWindow;
    subwindow->setWidget(new FM(mime, mimes, path));
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(path);
    subwindow->setWindowIcon(windowIcon());
    mdi->addSubWindow(subwindow);
}
