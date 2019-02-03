#ifndef CUSTOMACTIONSMANAGER_H
#define CUSTOMACTIONSMANAGER_H

#include <QSignalMapper>
#include <QSettings>
#include <QAction>
#include <QProcess>

/**
 * @class CustomActionsManager
 * @brief Manages custom actions
 * @author Michal Rost
 * @date 20.12.2012
 */
class CustomActionsManager : public QObject {
  Q_OBJECT
public:
  explicit CustomActionsManager(QSettings* settings,
                                QList<QAction*> *actionList,
                                QObject *parent = 0);
  QMultiHash<QString, QAction*>* getActions() const;
  QMultiHash<QString, QMenu*>* getMenus() const;
  QList<QAction*> *getActionList() const;
signals:
  void actionsLoaded();
  void actionsDeleted();
  void actionFinished();
  void actionMapped(const QString &s);
public slots:
  void execAction(const QString &cmd, const QString &path);
  void readActions();
  void freeActions();
protected slots:
  void onActionError(QProcess::ProcessError error);
  void onActionFinished(int ret);
protected:
  QSettings* settingsPtr;
  QList<QAction*> *actionListPtr;
  QMultiHash<QString, QAction*> *actions;
  QMultiHash<QString, QMenu*> *menus;
  QSignalMapper *mapper;
};

#endif // CUSTOMACTIONSMANAGER_H
