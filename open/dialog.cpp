/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "dialog.h"

iconHandler::iconHandler(QObject *parent) :
    QObject(parent)
{
    moveToThread(&t);
    t.start();
}

iconHandler::~iconHandler()
{
    t.wait();
    t.quit();
}

void iconHandler::requestIcon(QString icon)
{
    QMetaObject::invokeMethod(this, "findIcon", Q_ARG(QString, icon));
}

void iconHandler::findIcon(QString icon)
{
    if (icon.isEmpty()) { return; }
    QString result = Common::findIcon(icon);
    if (result.isEmpty()) {
        result = Common::findIcon("application-x-executable");
    }
    emit foundIcon(icon, result);
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , userInput(0)
    , appSuggestions(0)
{
    setAttribute(Qt::WA_QuitOnClose, true);
    setWindowTitle(tr("Open Application"));
    setMinimumWidth(350);
    setMaximumWidth(minimumWidth());
    setMinimumHeight(200);
    setMaximumHeight(minimumHeight());

    setupTheme();

    ih = new iconHandler();
    apps = FileUtils::getApplications();

    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    setLayout(containerLayout);
    containerLayout->setMargin(0);
    containerLayout->setSpacing(0);

    userInput = new QLineEdit(this);
    appSuggestions = new QListWidget(this);
    appSuggestions->setAlternatingRowColors(true);
    appSuggestions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    containerLayout->addWidget(userInput);
    containerLayout->addWidget(appSuggestions);

    userInput->setFocus();

    connect(ih, SIGNAL(foundIcon(QString,QString)), this, SLOT(handleFoundIcon(QString,QString)));
    connect(userInput, SIGNAL(textChanged(QString)), this, SLOT(handleUserInput(QString)));
    connect(userInput, SIGNAL(returnPressed()), this, SLOT(handleUserEnter()));
    connect(appSuggestions, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(handleAppClicked(QListWidgetItem*)));
}

Dialog::~Dialog()
{
}

void Dialog::handleUserInput(QString input)
{
    appSuggestions->clear();
    foreach (DesktopFile app, apps) {
        if (app.getName().compare("") == 0) { continue; }
        if (app.getName().contains(input, Qt::CaseInsensitive) ||
            app.getGenericName().contains(input, Qt::CaseInsensitive))
        {
            QListWidgetItem *appItem = new QListWidgetItem(appSuggestions);
            appItem->setText(app.getName());
            appItem->setData(LIST_EXE, app.getExec().replace("%u", "", Qt::CaseInsensitive).replace("%f", "", Qt::CaseInsensitive));
            appItem->setData(LIST_ICON, app.getIcon());
            appItem->setData(LIST_TERM, app.isTerminal());
            ih->requestIcon(app.getIcon());
        }
    }
}

void Dialog::handleUserEnter()
{
    if (appSuggestions->count()>0) {
        QString cmd = appSuggestions->item(0)->data(LIST_EXE).toString();
        if (appSuggestions->item(0)->data(LIST_TERM).toBool()) {
            cmd = QString("%1 -e  \"%2\"").arg(getTerminal()).arg(cmd);
        }
        QProcess::startDetached(cmd);
    } else {
        QProcess::startDetached(userInput->text());
    }
    close();
}

void Dialog::handleAppClicked(QListWidgetItem *app)
{
    if (!app) { return; }
    QProcess::startDetached(app->data(LIST_EXE).toString());
    close();
}

void Dialog::handleFoundIcon(QString icon, QString result)
{
    for (int i=0;i<appSuggestions->count();++i) {
        QListWidgetItem *item = appSuggestions->item(i);
        if (!item) { return; }
        if (item->data(LIST_ICON).toString() == icon) {
            item->setIcon(QIcon(result));
        }
    }
}

void Dialog::setupTheme()
{
    QString temp = QIcon::themeName();
    if (temp.isEmpty()  || temp == "hicolor") {
        QSettings settings(Common::configFile());
        temp = settings.value("fallbackTheme").toString();
    }
    if(temp.isEmpty() || temp == "hicolor") {
        if(QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0")) { // try gtk-2.0
            QSettings gtkFile(QDir::homePath() + "/.gtkrc-2.0",QSettings::IniFormat,this);
            temp = gtkFile.value("gtk-icon-theme-name").toString().remove("\"");
        }
        else { //try gtk-3.0
            QSettings gtkFile(QDir::homePath() + "/.config/gtk-3.0/settings.ini",QSettings::IniFormat,this);
            temp = gtkFile.value("gtk-fallback-icon-theme").toString().remove("\"");
        }
        //fallback
        if(temp.isNull()) {
            QStringList themes;
            themes << QString("%1/../share/icons/Tango").arg(qApp->applicationFilePath());
            themes << "/usr/share/icons/Tango" << "/usr/local/share/icons/Tango";
            themes << QString("%1/../share/icons/Adwaita").arg(qApp->applicationFilePath());
            themes << "/usr/share/icons/Adwaita" << "/usr/local/share/icons/Adwaita";
            themes << QString("%1/../share/icons/gnome").arg(qApp->applicationFilePath());
            themes << "/usr/share/icons/gnome" << "/usr/local/share/icons/gnome";
            themes << QString("%1/../share/icons/oxygen").arg(qApp->applicationFilePath());
            themes << "/usr/share/icons/oxygen" << "/usr/local/share/icons/oxygen";
            themes << QString("%1/../share/icons/hicolor").arg(qApp->applicationFilePath());
            themes << "/usr/share/icons/hicolor" << "/usr/local/share/icons/hicolor";
            for (int i=0;i<themes.size();++i) {
                if (QFile::exists(themes.at(i))) {
                    temp = QString(themes.at(i)).split("/").takeLast();
                    break;
                }
            }
        }
    }
    QIcon::setThemeName(temp);
}

QString Dialog::getTerminal()
{
    QSettings settings(Common::configFile());
    QString term = settings.value("term").toString();
    if (term.isEmpty()) { term = "xterm"; }
    return term;
}
