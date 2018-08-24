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

class QtFM : public QMainWindow
{
    Q_OBJECT

public:
    QtFM(QWidget *parent = nullptr);
    ~QtFM();

private:
    QMdiArea *mdi;
    MimeUtils *mimes;
    bool mime;

    QMenuBar *mBar;
    QStatusBar *sBar;
    QToolBar *navBar;
    QComboBox *pathEdit;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;

    QAction *tileAction;
    QAction *tabViewAction;

    QPushButton *backButton;
    QPushButton *upButton;
    QPushButton *homeButton;
    QPushButton *tileButton;

public slots:
    void newSubWindow(QString path);

private slots:
    void parseArgs();
    void setupConnections();
    void loadSettings();
    void writeSettings();
    void handleNewPath(QString path);
    void handleUpdatedDir(QString path);
};

#endif // QTFM_H
