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
    QString result;
    if (icon != "application-x-executable") { result = Common::findIcon(icon); }
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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowTitle(tr("Open Application"));
    setMinimumWidth(350);
    setupTheme();

    ih = new iconHandler();
    apps = FileUtils::getApplications();

    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    setLayout(containerLayout);
    containerLayout->setMargin(0);
    containerLayout->setSpacing(0);

    userInput = new QLineEdit(this);
    appSuggestions = new QListWidget(this);
    appSuggestions->setMinimumHeight(100);
    appSuggestions->setAlternatingRowColors(false);
    appSuggestions->hide();

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
    if (appSuggestions->isHidden()) {
        appSuggestions->show();
        QDesktopWidget wid;
        int screenWidth = wid.screen()->width();
        int screenHeight = wid.screen()->height();
        setGeometry((screenWidth/2)-(width()/2),(screenHeight/2)-(height()/2),width(),height());
    }
    foreach (DesktopFile app, apps) {
        if (app.getName().compare("") == 0 || app.noDisplay()) { continue; }
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
    QStringList additionalApps = Common::findApplications(input);
    for (int i=0;i<additionalApps.size();++i) {
        if (appExists(additionalApps.at(i))) { continue; }
        QListWidgetItem *appItem = new QListWidgetItem(appSuggestions);
        appItem->setText(additionalApps.at(i).split("/").takeLast());
        appItem->setData(LIST_EXE, additionalApps.at(i));
        appItem->setData(LIST_ICON, "application-x-executable");
        ih->requestIcon(appItem->data(LIST_ICON).toString());
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
    QString cmd = app->data(LIST_EXE).toString();
    if (app->data(LIST_TERM).toBool()) {
        cmd = QString("%1 -e  \"%2\"").arg(getTerminal()).arg(cmd);
    }
    QProcess::startDetached(cmd);
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

bool Dialog::appExists(QString exe)
{
    for (int i=0;i<appSuggestions->count();++i) {
        QListWidgetItem *item = appSuggestions->item(i);
        if (!item) { return false; }
        if (item->data(LIST_EXE).toString() == exe) {
            return true;
        }
    }
    return false;
}

void Dialog::setupTheme()
{
    Common::setupIconTheme();
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::Link, Qt::white);
    palette.setColor(QPalette::LinkVisited, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, Qt::white);
    palette.setColor(QPalette::HighlightedText, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    qApp->setPalette(palette);
}

QString Dialog::getTerminal()
{
    QSettings settings(Common::configFile());
    QString term = settings.value("term").toString();
    if (term.isEmpty()) { term = "xterm"; }
    return term;
}
