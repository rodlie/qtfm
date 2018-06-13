#include "applicationdock.h"
#include "fileutils.h"
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include "common.h"

ApplicationDock::ApplicationDock(QWidget *parent, Qt::WindowFlags flags)  : QDockWidget(parent)
{
    setWindowTitle(tr("Applications"));
    setWindowFlags(flags);

    appList = new QTreeWidget(this);
    appList->setIconSize(QSize(24, 24));
    appList->setAlternatingRowColors(false);
    appList->headerItem()->setText(0, tr("Application"));
    appList->setHeaderHidden(true);

    setWidget(appList);

    catNames.clear();
    catNames.insert("Development", QStringList() << "Programming");
    catNames.insert("Games", QStringList() << "Game");
    catNames.insert("Graphics", QStringList());
    catNames.insert("Internet", QStringList() << "Network" << "WebBrowser");
    catNames.insert("Multimedia", QStringList() << "AudioVideo" << "Video");
    catNames.insert("Office", QStringList());
    catNames.insert("Other", QStringList());
    catNames.insert("Settings", QStringList() << "System");
    catNames.insert("Utilities", QStringList() << "Utility");

    defaultIcon = QIcon::fromTheme("application-x-executable");

    connect(appList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(openApp(QTreeWidgetItem*,int)));

    QTimer::singleShot(1000, this, SLOT(populate()));
}

void ApplicationDock::refresh()
{
    QTimer::singleShot(1000, this, SLOT(populate()));
}

void ApplicationDock::populate()
{
    if (!isVisible()) { return; }
    appList->clear();

    // Create default application categories
    categories.clear();
    createCategories();

    // Load applications and create category tree list
    QList<DesktopFile> apps = FileUtils::getApplications();
    foreach (DesktopFile app, apps) {
        // Check for name
        if (app.getName().compare("") == 0 || app.noDisplay()) { continue; }

        // Find category
        QTreeWidgetItem* category = findCategory(app);

        // Create item from current mime
        QTreeWidgetItem *item = new QTreeWidgetItem(category);
        item->setIcon(0, FileUtils::searchAppIcon(app, defaultIcon));
        item->setText(0, app.getName());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(0, Qt::UserRole+1, app.getExec());
        item->setData(0, Qt::UserRole+2, app.isTerminal());

        // Register application
        QApplication::processEvents();
        applications.insert(app.getPureFileName(), item);
    }
}

void ApplicationDock::openApp(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col)
    if (!item) { return; }
    QString exe = item->data(0, Qt::UserRole+1).toString();
    bool useTerminal = item->data(0, Qt::UserRole+2).toBool();
    if (exe.isEmpty()) { return; }
    exe.replace("%u", "", Qt::CaseInsensitive);
    exe.replace("%f", "", Qt::CaseInsensitive);
    QString cmd;
    if (useTerminal) { cmd = QString("%1 -e \"%2\"").arg(getTerminal()).arg(exe); }
    else { cmd = exe; }
    qDebug() << "open app" << cmd;
    QProcess::startDetached(cmd);
}

QString ApplicationDock::getTerminal()
{
    QSettings settings(QString("%1/qtfm%2.conf").arg(Common::configDir()).arg(FM_MAJOR), QSettings::IniFormat);
    QString term = settings.value("term").toString();
    if (!term.isEmpty()) { return term; }
    return "xterm";
}

QTreeWidgetItem *ApplicationDock::findCategory(const DesktopFile &app)
{
    // Default category is 'Other'
    QTreeWidgetItem* category = categories.value("Other");

    // Try to find more suitable category
    foreach (QString name, catNames.keys()) {
        // Try category name
        if (app.getCategories().contains(name)) {
            category = categories.value(name);
            break;
        }

        // Try synonyms
        bool found = false;
        foreach (QString synonym, catNames.value(name)) {
            if (app.getCategories().contains(synonym)) {
                found = true;
                break;
            }
        }
        if (found) {
            category = categories.value(name);
            break;
        }
    }
    return category;
}

void ApplicationDock::createCategories()
{
    foreach (QString name, catNames.keys()) {
        // Find icon
        QIcon icon = QIcon::fromTheme("applications-" + name.toLower());

        // If icon not found, check synonyms
        if (icon.isNull()) {
            foreach (QString synonym, catNames.value(name)) {
                icon = QIcon::fromTheme("applications-" + synonym.toLower());
                break;
            }
        }

        // If icon still not found, retrieve default icon
        if (icon.isNull()) { icon = defaultIcon; }

        // Create category
        QTreeWidgetItem* category = new QTreeWidgetItem(appList);
        category->setText(0, name);
        category->setIcon(0, icon);
        category->setFlags(Qt::ItemIsEnabled);
        categories.insert(name, category);
    }
}
