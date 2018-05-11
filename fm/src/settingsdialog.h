#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "mimeutils.h"

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QTreeWidget>
#include <QToolButton>
#include <QSettings>
#include <QProgressBar>
#include <QComboBox>
#include <QGroupBox>

/**
 * @class SettingsDialog
 * @brief Represents dialog with application settings
 * @author Michal Rost
 * @date 18.12.2012
 */
class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  SettingsDialog(QList<QAction*> *actionList, QSettings* settings,
                 MimeUtils *mimeUtils, QWidget *parent = 0);
public slots:
  void accept();
  void loadMimes(int section);
  void readSettings();
  void readShortcuts();
  bool saveSettings();
protected slots:
  void addCustomAction();
  void delCustomAction();
  void infoCustomAction();
  void getIcon(QTreeWidgetItem *item, int column);
  void onActionChanged(QTreeWidgetItem *item, int column);
  void onMimeSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void updateMimeAssoc(QTreeWidgetItem* item);
  void showAppDialog();
  void removeAppAssoc();
  void moveAppAssocUp();
  void moveAppAssocDown();
protected:
  QWidget* createGeneralSettings();
  QWidget* createActionsSettings();
  QWidget* createShortcutSettings();
  QWidget* createMimeProgress();
  QWidget* createMimeSettings();

  MimeUtils* mimeUtilsPtr;

  QSettings* settingsPtr;
  QList<QAction*> *actionListPtr;

  QListWidget* selector;
  QStackedWidget* stack;

  QCheckBox* checkThumbs;
  QCheckBox* checkHidden;
  QCheckBox* checkTabs;
  QCheckBox* checkDelete;
  //QLineEdit* editTerm;
  QComboBox* cmbIconTheme;
  QComboBox* cmbDefaultMimeApps;

  QTreeWidget *actionsWidget;
  QToolButton *addButton;
  QToolButton *delButton;
  QToolButton *infoButton;
  QCheckBox* checkOutput;

  QTreeWidget* shortsWidget;

  QGroupBox* grpAssoc;
  QProgressBar* progressMime;
  QTreeWidget* mimesWidget;
  QListWidget* listAssoc;
};

#endif // SETTINGSDIALOG_H
