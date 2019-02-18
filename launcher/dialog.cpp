/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
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
 */

#include "dialog.h"
#include <QtConcurrent/qtconcurrentrun.h>

#include <QProcess>
#include <QSettings>
#include <QDesktopWidget>
#include <QDebug>
#include <QVBoxLayout>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , userInput(nullptr)
    , appSuggestions(nullptr)
    , iconCache(nullptr)
    , dirtyIconCache(false)
{
    setupTheme();
    setAttribute(Qt::WA_QuitOnClose, true);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowTitle(tr("Open Application"));
    setWindowIcon(QIcon::fromTheme("qtfm",
                                   QIcon(":/fm/images/qtfm.png")));
    setMinimumWidth(350);

    apps = FileUtils::getApplications();

    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    setLayout(containerLayout);
    containerLayout->setMargin(0);
    containerLayout->setSpacing(0);

    userInput = new QLineEdit(this);
    userInput->setPlaceholderText(tr("Find application ..."));
    userInput->setStyleSheet(QString("border: 3px solid white; border-radius: 5px; padding:5px;"));

    appSuggestions = new QListWidget(this);
    appSuggestions->setAlternatingRowColors(false);
    appSuggestions->hide();

    containerLayout->addWidget(userInput);
    containerLayout->addWidget(appSuggestions);

    userInput->setFocus();

    connect(this, SIGNAL(foundIcon(QString,QString)),
            this, SLOT(handleFoundIcon(QString,QString)));
    connect(userInput, SIGNAL(textChanged(QString)),
            this, SLOT(handleUserInput(QString)));
    connect(userInput, SIGNAL(returnPressed()),
            this, SLOT(handleUserEnter()));
    connect(appSuggestions, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(handleAppClicked(QListWidgetItem*)));

    iconCache = new QHash<QString, QIcon>;
    readIconCache();

    QTimer::singleShot(0, this, SLOT(doCenter()));
}

Dialog::~Dialog()
{
    if (dirtyIconCache) { writeIconCache(); }
}

void Dialog::handleUserInput(QString input)
{
    appSuggestions->clear();
    if (appSuggestions->isHidden()) {
        appSuggestions->show();
    }

    int ac = 0;
    foreach (DesktopFile app, apps) {
        if (app.getName().compare("") == 0 || app.noDisplay()) { continue; }
        if (app.getName().contains(input, Qt::CaseInsensitive) ||
            app.getGenericName().contains(input, Qt::CaseInsensitive))
        {
            QListWidgetItem *appItem = new QListWidgetItem(appSuggestions);
            appItem->setText(app.getName());
            appItem->setData(LIST_EXE,
                             app.getExec()
                             .replace("%u", "", Qt::CaseInsensitive)
                             .replace("%f", "", Qt::CaseInsensitive));
            appItem->setData(LIST_ICON, app.getIcon());
            appItem->setData(LIST_TERM, app.isTerminal());
            if (iconCache->contains(app.getIcon())) {
                appItem->setIcon(iconCache->value(app.getIcon()));
            }
            else {
                QtConcurrent::run(this,
                                  &Dialog::findIcon,
                                  app.getIcon());
            }
            ac++;
        }
    }
    QStringList additionalApps = Common::findApplications(input);
    for (int i=0;i<additionalApps.size();++i) {
        if (appExists(additionalApps.at(i)) ||
            appExists(additionalApps.at(i).split("/").takeLast())) { continue; }
        QListWidgetItem *appItem = new QListWidgetItem(appSuggestions);
        appItem->setText(additionalApps.at(i).split("/").takeLast());
        appItem->setData(LIST_EXE, additionalApps.at(i));
        appItem->setData(LIST_ICON, "application-x-executable");
        appItem->setData(LIST_TERM, true);
        if (iconCache->contains(appItem->data(LIST_ICON).toString())) {
            appItem->setIcon(iconCache->value(appItem->data(LIST_ICON).toString()));
        }
        else {
            QtConcurrent::run(this,
                              &Dialog::findIcon,
                              appItem->data(LIST_ICON).toString());
        }
        ac++;
    }

    QDesktopWidget wid;
    int screenHeight = wid.screen()->height()-100;
    int as = (ac*30)+20;
    if (as>screenHeight) { as = screenHeight; }
    setMinimumHeight(as);
    setMaximumHeight(as);
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
    if (icon.isEmpty()) { return; }
    if (!iconCache->contains(icon) && !QIcon(result).isNull()) {
        iconCache->insert(icon, QIcon(result));
        dirtyIconCache = true;
    }
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
    Common::setupIconTheme(qApp->applicationFilePath());
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
    QSettings settings(Common::configFile(), QSettings::IniFormat);
    return settings.value("term", "xterm").toString();
}

void Dialog::doCenter(bool horiz)
{
    QDesktopWidget wid;
    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();
    int newWidth = (screenWidth/2)-(width()/2);
    int newHeight = (screenHeight/2)-(height()/2);
    if (horiz) { newHeight = screenHeight/height(); }
    setGeometry(newWidth,
                newHeight,
                width(),
                height());
}

void Dialog::readIconCache()
{
    QFile fileIcons(QString("%1/applications_icons.cache")
                    .arg(Common::configDir()));
    if (fileIcons.open(QIODevice::ReadOnly)) {
        QDataStream out(&fileIcons);
        out >> *iconCache;
        fileIcons.close();
    }
}

void Dialog::writeIconCache()
{
    QFile fileIcons(QString("%1/applications_icons.cache")
                    .arg(Common::configDir()));
    if(fileIcons.size() > 10000000) { fileIcons.remove(); }
    else {
        if (fileIcons.open(QIODevice::WriteOnly)) {
            QDataStream out(&fileIcons);
            out.setDevice(&fileIcons);
            out << *iconCache;
            fileIcons.close();
        }
    }
}

void Dialog::findIcon(QString icon)
{
    if (iconCache->contains(icon)) { return; }
    qDebug() << "find icon"<<icon;
    if (icon.isEmpty()) { return; }
    QString result;
    if (icon != "application-x-executable") {
        result = Common::findIcon(qApp->applicationFilePath(),
                                  QIcon::themeName(), icon);
    }
    if (result.isEmpty()) {
        result = Common::findIcon(qApp->applicationFilePath(),
                                  QIcon::themeName(), "application-x-executable");
    }
    emit foundIcon(icon, result);
}
