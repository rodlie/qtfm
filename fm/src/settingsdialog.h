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
  SettingsDialog(QList<QAction*> *actionList,
                 QSettings* settings,
                 MimeUtils *mimeUtils,
                 QWidget *parent = Q_NULLPTR);

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
  void clearCustomAction();
  void getIcon(QTreeWidgetItem *item,
               int column);
  void onActionChanged(QTreeWidgetItem *item,
                       int column);
  void onMimeSelected(QTreeWidgetItem* current,
                      QTreeWidgetItem* previous);
  void updateMimeAssoc(QTreeWidgetItem* item);
  void showAppDialog();
  void removeAppAssoc();
  void moveAppAssocUp();
  void moveAppAssocDown();
  void restartToApply(int triggered);
  void restartToApply(bool triggered);
  void filterMimes(QString filter);

protected:
  QWidget* createGeneralSettings();
  QWidget *createAppearanceSettings();
  QWidget* createActionsSettings();
  QWidget* createShortcutSettings();
  QWidget* createMimeSettings();
  QWidget* createSystraySettings();
  QWidget* createAdvSettings();
  QWidget* createSessionSettings();
  MimeUtils* mimeUtilsPtr;
  QSettings* settingsPtr;
  QList<QAction*> *actionListPtr;
  QListWidget* selector;
  QStackedWidget* stack;
  QCheckBox* checkDelete;
  QComboBox* comboDAD;
  QComboBox* comboDADctl;
  QComboBox* comboDADshift;
  QComboBox* comboDADalt;
  QLineEdit* editTerm;
  QComboBox* cmbIconTheme;
  QComboBox* cmbDefaultMimeApps;
  QComboBox* comboSingleClick;
  QCheckBox* showTerminalButton;
  QCheckBox* showHomeButton;
  QCheckBox* showNewTabButton;
  QCheckBox* checkDarkTheme;
  QCheckBox* checkFileColor;
  QCheckBox* checkPathHistory;
  QTreeWidget *actionsWidget;
  QToolButton *addButton;
  QToolButton *delButton;
  QToolButton *infoButton;
  QToolButton *clearButton;
  QCheckBox* checkOutput;
  QTreeWidget* shortsWidget;
  QGroupBox* grpAssoc;
  QTreeWidget* mimesWidget;
  QListWidget* listAssoc;
  QCheckBox* checkTrayNotify;
  QCheckBox* checkAudioCD;
  QCheckBox* checkAutoMount;
  QCheckBox* checkDVD;
  QCheckBox* checkWindowTitlePath;
  QLineEdit* editCopyX;
  QLineEdit* editCopyTS;
  QCheckBox* checkSaveSession;
};

#endif // SETTINGSDIALOG_H
