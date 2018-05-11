#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QHBoxLayout>

/**
 * @class AboutDialog
 * @brief Show info about this application
 * @author Michal Rost
 * @date 1.5.2013
 */
class AboutDialog : public QDialog {
  Q_OBJECT
public:
  explicit AboutDialog(QWidget *parent = 0);
};

#endif // ABOUTDIALOG_H
