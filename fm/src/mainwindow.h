/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QLabel>
#include <QStackedWidget>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QSignalMapper>
#include <QToolBar>
#include <QItemDelegate>
#include <QStyledItemDelegate>

#include "mymodel.h"
#include "bookmarkmodel.h"
#include "progressdlg.h"
#include "propertiesdlg.h"
#include "icondlg.h"
#include "tabbar.h"
#include "fileutils.h"
#include "mimeutils.h"
#include "customactionsmanager.h"

// libdisks
#ifndef NO_UDISKS
#include <disks.h>
#endif

// service
#ifndef NO_DBUS
#include "service.h"
#endif

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

//---------------------------------------------------------------------------------
class IconViewDelegate : public QStyledItemDelegate
{
private: // workaround for QTBUG
    mutable bool _isEditing;
    mutable QModelIndex _index;
protected: // workaround for QTBUG
    bool eventFilter(QObject * object, QEvent * event)
    {
        QWidget *editor = qobject_cast<QWidget*>(object);
        if(editor && event->type() == QEvent::KeyPress) {
            if(static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape){
                _isEditing = false;
                _index = QModelIndex();
            }
        }
        return QStyledItemDelegate::eventFilter(editor, event);
    }
public:
    void setEditorData(QWidget * editor, const QModelIndex & index) const
    { // workaround for QTBUG
        _isEditing = true;
        _index = index;
        QStyledItemDelegate::setEditorData(editor, index);
    }
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
    { // workaround for QTBUG
        QStyledItemDelegate::setModelData(editor, model, index);
        _isEditing = false;
        _index = QModelIndex();
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        int width = qMax(iconsize.width(), option.fontMetrics.averageCharWidth() * 13);
        QRect txtRect(0, 0, width, option.rect.height());
        QSize txtsize = option.fontMetrics.boundingRect(txtRect,
                                                        Qt::AlignCenter|Qt::TextWrapAnywhere,
                                                        index.data().toString()).size();
        QSize size(width+8, txtsize.height()+iconsize.height()+8+8);
        return size;
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect iconRect(item.left()+(item.width()/2)-(iconsize.width()/2),
                       item.top()+4+4, iconsize.width(), iconsize.height());
        QRect txtRect(item.left()+4, item.top()+iconsize.height()+4+4,
                      item.width()-8, item.height()-iconsize.height()-4);
        QBrush txtBrush = qvariant_cast<QBrush>(index.data(Qt::ForegroundRole));
        bool isSelected = option.state & QStyle::State_Selected;
        bool isEditing = _isEditing && index==_index;

        /*QStyleOptionViewItem opt = option;
        initStyleOption(&opt,index);
        opt.decorationAlignment |= Qt::AlignCenter;
        opt.displayAlignment    |= Qt::AlignCenter;
        opt.decorationPosition   = QStyleOptionViewItem::Top;
        opt.features |= QStyleOptionViewItem::WrapText;
        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem,&opt,painter);*/

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);

        if (isSelected && !isEditing) {
            QPainterPath path;
            QRect frame(item.left(),item.top()+4, item.width(), item.height()-4);
            path.addRoundRect(frame, 15, 15);
            //  path.addRect(frame);
            painter->setOpacity(0.5);
            painter->fillPath(path, option.palette.highlight());
            painter->setOpacity(1.0);
        }

        painter->drawPixmap(iconRect, icon.pixmap(iconsize.width(),iconsize.height()));

        if (isEditing) { return; }
        if (isSelected) { painter->setPen(option.palette.highlightedText().color()); }
        else { painter->setPen(txtBrush.color()); }

        painter->drawText(txtRect,
                          Qt::AlignCenter|Qt::AlignVCenter|Qt::TextWrapAnywhere,
                          index.data().toString());
    }
};
class IconListDelegate : public QItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect txtRect(item.left()+iconsize.width()+5,
                      item.top(), item.width(), item.height());
        QSize txtsize = option.fontMetrics.boundingRect(txtRect,
                                                        Qt::AlignLeft|Qt::AlignVCenter,
                                                        index.data().toString()).size();
        return QSize(txtsize.width()+iconsize.width()+10,iconsize.height());
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);
        QRect item = option.rect;
        QRect iconRect(item.left(), item.top(), iconsize.width(), iconsize.height());
        QRect txtRect(item.left()+iconsize.width()+5,
                      item.top(), item.width()-iconsize.width()-5, item.height()-2);
        QBrush txtBrush = qvariant_cast<QBrush>(index.data(Qt::ForegroundRole));

        if (option.state & QStyle::State_Selected) {
            QPainterPath path;
            path.addRect(txtRect);
            painter->fillPath(path, option.palette.highlight());
        }

        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        } else { painter->setPen(txtBrush.color()); }

        painter->drawPixmap(iconRect, icon.pixmap(iconsize.width(),iconsize.height()));
        painter->drawText(txtRect, Qt::AlignLeft|Qt::AlignVCenter, index.data().toString());
    }
};
//---------------------------------------------------------------------------------

/**
 * @class MainWindow
 * @brief The main window class
 * @author Wittefella
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    myModel *modelList;

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void treeSelectionChanged(QModelIndex,QModelIndex);
    void listSelectionChanged(const QItemSelection, const QItemSelection);
    void listDoubleClicked(QModelIndex);
    void lateStart();
    void goUpDir();
    void goBackDir();
    void goHomeDir();
    void deleteFile();
    void trashFile();
    void cutFile();
    void copyFile();
    bool cutCopyFile(const QString &source, QString dest, qint64 totalSize, bool cut);
    bool pasteFiles(const QList<QUrl> &files, const QString &newPath, const QStringList &cutList);
    bool linkFiles(const QList<QUrl> &files, const QString &newPath);
    void newDir();
    void newFile();
    void pathEditChanged(QString);
    void terminalRun();
    void executeFile(QModelIndex, bool);
    void runFile();
    void openFile();
    void clipboardChanged();
    void toggleDetails();
    void toggleHidden();
    void toggleIcons();
    void toggleSortBy(QAction* action);
    void toggleSortOrder();
    void setSortOrder(Qt::SortOrder order);
    void setSortColumn(QAction *columnAct);
    void toggleThumbs();
    void addBookmarkAction();
    void addSeparatorAction();
    void delBookmark();
    void editBookmark();
    void toggleWrapBookmarks();
    void showEditDialog();
    bool copyFolder(const QString &srcFolder, const QString &dstFolder, qint64, bool);
    void renameFile();
    void actionMapper(QString);
    void folderPropertiesLauncher();
    void bookmarkClicked(QModelIndex);
    void bookmarkPressed(QModelIndex);
    void bookmarkShortcutTrigger();
    void contextMenuEvent(QContextMenuEvent *);
    void toggleLockLayout();
    void dragLauncher(const QMimeData *data, const QString &newPath, Common::DragMode dragMode);
    void pasteLauncher(const QMimeData *data, const QString &newPath, const QStringList &cutList);
    void pasteLauncher(const QList<QUrl> &files, const QString &newPath, const QStringList &cutList, bool link = false);
    void pasteClipboard();
    void progressFinished(int,QStringList);
    void listItemClicked(QModelIndex);
    void listItemPressed(QModelIndex);
    void tabChanged(int index);
    void newWindow();
    void openTab();
    void tabsOnTop();
    int addTab(QString path);
    void clearCutItems();
    void zoomInAction();
    void zoomOutAction();
    void focusAction();
    void openFolderAction();
    void exitAction();
    void dirLoaded();
    void thumbUpdate(QModelIndex);
    void addressChanged(int,int);
    void loadSettings(bool wState = true, bool hState = true, bool tabState = true, bool thumbState = true);
    void firstRunBookmarks(bool isFirstRun);
    void loadBookmarks();
    void writeBookmarks();
    void handleBookmarksChanged();
    void firstRunCustomActions(bool isFirstRun);
    void showAboutBox();

signals:
    void updateCopyProgress(qint64, qint64, QString);
    void copyProgressFinished(int,QStringList);

private slots:
    void readShortcuts();
    void selectApp();
    void selectAppForFiles();
    void openInApp();
    void updateGrid();
    // libdisks
#ifndef NO_UDISKS
    void populateMedia();
    void handleMediaMountpointChanged(QString path, QString mountpoint);
    int mediaBookmarkExists(QString path);
    void handleMediaAdded(QString path);
    void handleMediaRemoved(QString path);
    void handleMediaChanged(QString path, bool present);
    void handleMediaUnmount();
    void handleMediaEject();
#endif
    void clearCache();
    void handlePathRequested(QString path);
private:
    void createActions();
    void createActionIcons();
    void createMenus();
    void createToolBars();
    void writeSettings();
    void recurseFolder(QString path, QString parent, QStringList *);
    int showReplaceMsgBox(const QFileInfo &f1, const QFileInfo &f2);
    QMenu* createOpenWithMenu();

    int zoom;
    int zoomTree;
    int zoomList;
    int zoomDetail;
    int zoomBook;
    int currentView;        // 0=list, 1=icons, 2=details
    int currentSortColumn;  // 0=name, 1=size, 3=date
    Qt::SortOrder currentSortOrder;

    QCompleter *customComplete;
    tabBar *tabs;
    MimeUtils *mimeUtils;

    myProgressDialog * progress;
    PropertiesDialog * properties;
    QSettings *settings;
    QDockWidget *dockTree;
    QDockWidget *dockBookmarks;
    QVBoxLayout *mainLayout;
    QStackedWidget *stackWidget;
    QTreeView *tree;
    QTreeView *detailTree;
    QListView *list;
    QListView *bookmarksList;
    QComboBox *pathEdit;

    QString term;
    QFileInfo curIndex;
    QModelIndex backIndex;

    QSortFilterProxyModel *modelTree;
    QSortFilterProxyModel *modelView;

    bookmarkmodel *modelBookmarks;
    QItemSelectionModel *treeSelectionModel;
    QItemSelectionModel *listSelectionModel;
    QStringList mounts;

    QList<QIcon> *actionIcons;
    QList<QAction*> *actionList;
    QList<QAction*> bookmarkActionList;
    CustomActionsManager* customActManager;

    //QToolBar *editToolBar;
    //QToolBar *viewToolBar;
    QToolBar *navToolBar;
    QToolBar *addressToolBar;
    QToolBar *menuToolBar;
    QStatusBar * status;
    QLabel * statusSize;
    QLabel * statusName;
    QLabel * statusDate;
    QString startPath;

    QAction *closeAct;
    QAction *exitAct;
    QAction *upAct;
    QAction *backAct;
    QAction *homeAct;
    QAction *hiddenAct;
    QAction *filterAct;
    QAction *detailAct;
    QAction *addBookmarkAct;
    QAction *addSeparatorAct;
    QAction *delBookmarkAct;
    QAction *editBookmarkAct;
    QAction *wrapBookmarksAct;
    QAction *deleteAct;
    QAction *iconAct;
    QActionGroup *sortByActGrp;
    QAction *sortNameAct;
    QAction *sortDateAct;
    QAction *sortSizeAct;
    QAction *sortAscAct;
    QAction *newDirAct;
    QAction *newFileAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *settingsAct;
    QAction *renameAct;
    QAction *terminalAct;
    QAction *openAct;
    QAction *runAct;
    QAction *thumbsAct;
    QAction *folderPropertiesAct;
    QAction *lockLayoutAct;
    //QAction *escapeAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *focusAddressAct;
    QAction *focusTreeAct;
    QAction *focusBookmarksAct;
    QAction *focusListAct;
    QAction *openFolderAct;
    QAction *newWinAct;
    QAction *openTabAct;
    QAction *closeTabAct;
    QAction *tabsOnTopAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
#ifndef NO_UDISKS
    QAction *mediaUnmountAct;
    QAction *mediaEjectAct;
#endif
    QAction *trashAct;
    QAction *clearCacheAct;
    // libdisks
#ifndef NO_UDISKS
    Disks *disks;
#endif
    QString trashDir;

#ifndef NO_DBUS
    qtfm *service;
#endif

    bool pathHistory;
    bool showPathInWindowTitle;

    IconViewDelegate *ivdelegate;
    IconListDelegate *ildelegate;

    // copy of original new filename
    QString copyXof;
    // custom timestamp for copy of
    QString copyXofTS;
};

//---------------------------------------------------------------------------------
// Subclass QSortFilterProxyModel and override 'filterAcceptsRow' to only show
// directories in tree and not files.
//---------------------------------------------------------------------------------
class mainTreeFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};


//---------------------------------------------------------------------------------
// Subclass QSortFilterProxyModel and override 'lessThan' for sorting in list/details views
//---------------------------------------------------------------------------------
class viewsSortProxyModel : public QSortFilterProxyModel
{
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};


//---------------------------------------------------------------------------------
// Subclass QCompleter so we can use the SortFilterProxyModel above for the address bar.
//---------------------------------------------------------------------------------
class myCompleter : public QCompleter
{
public:
    QString pathFromIndex(const QModelIndex& index) const;
    QStringList splitPath(const QString& path) const;
};

//---------------------------------------------------------------------------------
// Global functions.
//---------------------------------------------------------------------------------
QString formatSize(qint64);
QString getDriveInfo(QString);
QString gGetMimeType(QString);
#endif
