#ifndef QTFM_H
#define QTFM_H

#include <QMainWindow>
#include <QMdiArea>
#include <QComboBox>
#include <QToolBar>
#include <QPushButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>

#include "mimeutils.h"
#include "fm.h"

class QtFM : public QMainWindow
{
    Q_OBJECT

public:
    QtFM(QWidget *parent = Q_NULLPTR);
    ~QtFM();

private:
    QMdiArea *mdi;
    MimeUtils *mimes;

    QMenuBar *mBar;
    QStatusBar *sBar;
    QToolBar *navBar;
    QComboBox *pathEdit;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;

    QAction *tileAction;
    QAction *tabViewAction;
    QAction *newTabAction;

    QPushButton *backButton;
    QPushButton *upButton;
    QPushButton *homeButton;
    QPushButton *tileButton;

public slots:
    void newSubWindow(bool triggered);
    void newSubWindow(QString path = QDir::homePath());

private slots:
    void parseArgs();
    void setupConnections();
    void loadSettings();
    void writeSettings();
    void handleNewPath(QString path);
    void handleUpdatedDir(QString path);
    void handleTabActivated(QMdiSubWindow *tab);
    void handleOpenFile(const QString &file);
    void handlePreviewFile(const QString &file);
    void refreshPath(FM* fm);
    void pathEditChanged(const QString &path);
};

#endif // QTFM_H
