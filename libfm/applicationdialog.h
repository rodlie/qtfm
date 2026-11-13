#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include "desktopfile.h"

#include <QDialog>
#include <QTreeWidgetItem>
#include <QMap>
#include <QStringList>
#include <QLineEdit>
#include <QCheckBox>
#include <QIcon>

class ApplicationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ApplicationDialog(bool enable_launcher = true,
                             QWidget *parent = Q_NULLPTR);
  QString getCurrentLauncher() const;
  // file type the application is chosen for; with offerDefault, a check box
  // "Use as default" is shown (see useAsDefault())
  void setMimeType(const QString &mime, bool offerDefault);
  bool useAsDefault() const;

public slots:
  void accept() override;

private slots:
  void populate();
  void browseCommand();
  void customCommandEdited(const QString &text);

protected slots:
  void updateCommand(QTreeWidgetItem* current,
                     QTreeWidgetItem* previous);

protected:
  QTreeWidget *appList;
  QLineEdit *edtCommand;
  QLineEdit *edtCustom;      // command of an application that is not listed
  QCheckBox *chkTerminal;    // run that command in a terminal
  QCheckBox *chkDefault;     // use the chosen application as default
  QString mimeType;
  QString chosenLauncher;    // result of accept()
  DesktopFile result;
  QIcon defaultIcon;
  QMap<QString, QStringList> catNames;
  QMap<QString, QTreeWidgetItem*> categories;
  QMap<QString, QTreeWidgetItem*> applications;
  QTreeWidgetItem* findCategory(const DesktopFile &app);
  void createCategories();
};

#endif // APPLICATIONDIALOG_H
