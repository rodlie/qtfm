#include "customactionsmanager.h"
#include "processdialog.h"
#include <QMessageBox>
#include <QMenu>
#include <QTimer>

/**
 * @brief Creates custom action manager
 * @param settings
 * @param actionList
 * @param parent
 */
CustomActionsManager::CustomActionsManager(QSettings* settings,
                                           QList<QAction*> *actionList,
                                           QObject *parent) : QObject(parent) {
  // Store pointers
  this->settingsPtr = settings;
  this->actionListPtr = actionList;

  // Create data
  this->actions = new QMultiHash<QString,QAction*>;
  this->menus = new QMultiHash<QString,QMenu*>;
  this->mapper = new QSignalMapper(this);
  connect(mapper, SIGNAL(mapped(QString)), SIGNAL(actionMapped(QString)));
}
//---------------------------------------------------------------------------

/**
 * @brief Removes all custom actions
 */
void CustomActionsManager::freeActions() {

  // Delete actions
  foreach (QAction *action, *actionListPtr) {
    if (actions->values().contains(action)) {
      actionListPtr->removeOne(action);
      delete action;
    }
  }

  // Delete entries from custom menus
  QList<QMenu*> temp = menus->values();
  foreach (QMenu* menu, temp) {
    delete menu;
  }

  // Clear lists
  actions->clear();
  menus->clear();
  emit actionsDeleted();
}
//---------------------------------------------------------------------------

/**
 * @brief Reads actions from QSettings
 */
void CustomActionsManager::readActions() {

  // Read keys
  settingsPtr->beginGroup("customActions");
  QStringList keys = settingsPtr->childKeys();

  // Read actions
  for (int i = 0; i < keys.count(); ++i) {

    // Add action key in reverse order
    keys.insert(i, keys.takeLast());

    // temp.at(0) - FileType
    // temp.at(1) - Text
    // temp.at(2) - Icon
    // temp.at(3) - Command
    QStringList temp(settingsPtr->value(keys.at(i)).toStringList());

    // Create new action and read it
    QAction *act = new QAction(QIcon::fromTheme(temp.at(2)), temp.at(1), this);
    mapper->setMapping(act, temp.at(3));
    connect(act, SIGNAL(triggered()), mapper, SLOT(map()));
    actionListPtr->append(act);

    // Parse types which are connected with current action
    QStringList types = temp.at(0).split(",");
    foreach (QString type, types) {
      QStringList children(temp.at(1).split(" / "));
      if (children.count() > 1) {
        QMenu* parent = 0;
        act->setText(children.at(1));
        foreach (QMenu *subMenu, menus->values(type)) {
          if (subMenu->title() == children.at(0)) {
            parent = subMenu;
          }
        }
        if (parent == 0) {
          parent = new QMenu(children.at(0));
          menus->insert(type, parent);
        }
        parent->addAction(act);
        actions->insert("null", act);
      } else {
        actions->insert(type, act);
      }
    }
  }
  settingsPtr->endGroup();
  emit actionsLoaded();
}
//---------------------------------------------------------------------------

/**
 * @brief Returns custom actions
 * @return actions
 */
QMultiHash<QString, QAction*>* CustomActionsManager::getActions() const {
  return actions;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns custom menus
 * @return menus
 */
QMultiHash<QString, QMenu*>* CustomActionsManager::getMenus() const {
  return menus;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns action list pointer
 * @return
 */
QList<QAction*>* CustomActionsManager::getActionList() const {
  return actionListPtr;
}
//---------------------------------------------------------------------------

/**
 * @brief Executes action
 * @param cmd command
 * @param path path to file
 */
void CustomActionsManager::execAction(const QString &cmd, const QString &path) {

  // Retrieve executable name from splitted command, the rest is arguments
  QStringList temp = cmd.split(" ");
  QString exec = temp.at(0);
  temp.removeAt(0);

  // Fix rest of command
  temp.replaceInStrings("\\","\ ");

  // Create new custom process
  QProcess *p = new QProcess();
  p->setWorkingDirectory(path);

  // Create process dialog
  if (settingsPtr->value("showActionOutput", true).toBool()) {
    new ProcessDialog(p, exec, qobject_cast<QWidget*>(parent()));
  }

  // Connect process
  connect(p, SIGNAL(finished(int)), this, SLOT(onActionFinished(int)));
  connect(p, SIGNAL(error(QProcess::ProcessError)), this,
          SLOT(onActionError(QProcess::ProcessError)));

  // Execute process
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (exec.at(0) == '|') {
    exec.remove(0, 1);
    env.insert("QTFM", "1");
    p->setProcessEnvironment(env);
  }
  p->start(exec, temp);
}
//---------------------------------------------------------------------------

/**
 * @brief Displays error message
 * @param error
 */
void CustomActionsManager::onActionError(QProcess::ProcessError error) {
  QProcess* process = qobject_cast<QProcess*>(sender());
  QMessageBox::warning(NULL, "Error", process->errorString());
  onActionFinished(0);
}
//---------------------------------------------------------------------------

/**
 * @brief Triggered when action is finished
 * @param ret
 */
void CustomActionsManager::onActionFinished(int ret) {

  QProcess* process = qobject_cast<QProcess*>(sender());
  if (process->processEnvironment().contains("QTFM")) {
    QString output = process->readAllStandardError();
    if (!output.isEmpty()) {
      QMessageBox::warning(NULL, tr("Error - Custom action"), output);
    }
    output = process->readAllStandardOutput();
    if (!output.isEmpty()) {
      QMessageBox::information(NULL, tr("Output - Custom action"), output);
    }
  }

  // Updates file sizes
  QTimer::singleShot(100, this, SIGNAL(actionFinished()));
  process->deleteLater();
}
//---------------------------------------------------------------------------
