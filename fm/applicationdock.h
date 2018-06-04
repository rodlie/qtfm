#ifndef APPLICATIONDOCK_H
#define APPLICATIONDOCK_H

#include "desktopfile.h"

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class ApplicationDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ApplicationDock(QWidget *parent = NULL, Qt::WindowFlags flags = Qt::WindowFlags());

public slots:
    void refresh();
private slots:
    void populate();
    void openApp(QTreeWidgetItem *item, int col);
    QString getTerminal();

protected:
    QTreeWidget* appList;
//    QLineEdit* edtCommand;
  //  DesktopFile result;
    QIcon defaultIcon;
    QMap<QString, QStringList> catNames;
    QMap<QString, QTreeWidgetItem*> categories;
    QMap<QString, QTreeWidgetItem*> applications;
    QTreeWidgetItem* findCategory(const DesktopFile &app);
    void createCategories();
};

#endif // APPLICATIONDOCK_H
