#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include "disks.h"
#include "mimeutils.h"

class SysTray : public QObject
{
    Q_OBJECT
public:
    explicit SysTray(QObject *parent = Q_NULLPTR);

private:
    QSystemTrayIcon *disktray;
    QMenu *menu;
    Disks *man;
    bool showNotifications;
    MimeUtils* mimeUtilsPtr;
    bool autoMount;

private slots:
    void loadSettings();
    void generateContextMenu();
    void disktrayActivated(QSystemTrayIcon::ActivationReason reason);
    void handleDisktrayMessageClicked();
    void showMessage(QString title,
                     QString message);
    void handleContextMenuAction();
    void handleDeviceError(QString path,
                           QString error);
    void handleDeviceMediaChanged(QString path,
                                  bool media);
    void handleDeviceMountpointChanged(QString path,
                                       QString mountpoint);
    void openMountpoint(QString mountpoint);
    void handleFoundNewDevice(QString path);
    void handleShowHideDisktray();
};

#endif // SYSTRAY_H
