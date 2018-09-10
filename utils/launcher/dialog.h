/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QDebug>
#include <QList>
#include <QProcess>
#include <QThread>
#include <QSettings>
#include <QDesktopWidget>

#include "properties.h"
#include "desktopfile.h"
#include "fileutils.h"
#include "common.h"

#define LIST_EXE Qt::UserRole+1
#define LIST_ICON Qt::UserRole+2
#define LIST_TERM Qt::UserRole+3

class iconHandler : public QObject
{
    Q_OBJECT

public:
    explicit iconHandler(QObject *parent = Q_NULLPTR);
    ~iconHandler();

private:
    QThread t;

signals:
    void foundIcon(QString icon, QString result);

public slots:
    void requestIcon(QString icon);

private slots:
    void findIcon(QString icon);
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
   explicit Dialog(QWidget *parent = Q_NULLPTR);
   ~Dialog();

private:
    QLineEdit *userInput;
    QListWidget *appSuggestions;
    QList<DesktopFile> apps;
    iconHandler *ih;

private slots:
    void handleUserInput(QString input);
    void handleUserEnter();
    void handleAppClicked(QListWidgetItem *app);
    void handleFoundIcon(QString icon, QString result);
    bool appExists(QString exe);
    void setupTheme();
    QString getTerminal();
    void doCenter();
};

#endif // DIALOG_H
