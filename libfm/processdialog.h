#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QListWidget>

/**
 * @class ProcessDialog
 * @brief Represents dialog which is shown during custom action
 * @author Michal Rost
 * @date 30.12.2012
 */
class ProcessDialog : public QDialog {
  Q_OBJECT
public:
  explicit ProcessDialog(QProcess* proc, const QString &procName = "",
                         QWidget *parent = Q_NULLPTR);
protected slots:
  void onProcFinished();
  void onProcStarted();
  void onProcStdOut();
protected:
  QProcess* procPtr;
  QListWidget* output;
};

#endif // PROCESSDIALOG_H
