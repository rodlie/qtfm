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

void Dialog::setupTheme()
{
    Common::setupIconTheme();
}

QString Dialog::getTerminal()
{
    QSettings settings(Common::configFile());
    QString term = settings.value("term").toString();
    if (term.isEmpty()) { term = "xterm"; }
    return term;
}
