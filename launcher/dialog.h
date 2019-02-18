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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QList>
#include <QHash>

#include "properties.h"
#include "desktopfile.h"
#include "fileutils.h"
#include "common.h"

#define LIST_EXE Qt::UserRole+1
#define LIST_ICON Qt::UserRole+2
#define LIST_TERM Qt::UserRole+3

class Dialog : public QDialog
{
    Q_OBJECT

public:
   explicit Dialog(QWidget *parent = nullptr);
   ~Dialog();

signals:
    void foundIcon(QString icon, QString result);

private:
    QLineEdit *userInput;
    QListWidget *appSuggestions;
    QList<DesktopFile> apps;
    QHash<QString, QIcon> *iconCache;
    bool dirtyIconCache;

private slots:
    void handleUserInput(QString input);
    void handleUserEnter();
    void handleAppClicked(QListWidgetItem *app);
    void handleFoundIcon(QString icon, QString result);
    bool appExists(QString exe);
    void setupTheme();
    QString getTerminal();
    void doCenter(bool horiz = true);
    void readIconCache();
    void writeIconCache();
    void findIcon(QString icon);
};

#endif // DIALOG_H
