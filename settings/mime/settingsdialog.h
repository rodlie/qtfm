#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "mimeutils.h"

#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QTreeWidget>
#include <QToolButton>
#include <QSettings>
#include <QComboBox>
#include <QGroupBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QSettings* settings,
                   MimeUtils *mimeUtils,
                   QWidget *parent = Q_NULLPTR);

public slots:
    void accept();
    void loadMimes();
    void readSettings();
    bool saveSettings();

protected slots:
    void getIcon(QTreeWidgetItem *item,
                 int column);
    void onMimeSelected(QTreeWidgetItem* current,
                        QTreeWidgetItem* previous);
    void updateMimeAssoc(QTreeWidgetItem* item);
    void showAppDialog();
    void removeAppAssoc();
    void moveAppAssocUp();
    void moveAppAssocDown();
    void filterMimes(QString filter);

protected:
    QWidget* createMimeSettings();
    MimeUtils* mimeUtilsPtr;
    QSettings* settingsPtr;
    QComboBox* cmbIconTheme;
    QComboBox* cmbDefaultMimeApps;
    QGroupBox* grpAssoc;
    QTreeWidget* mimesWidget;
    QListWidget* listAssoc;
};

#endif // SETTINGSDIALOG_H
