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
   explicit Dialog(QWidget *parent = Q_NULLPTR);
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
