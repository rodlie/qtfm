/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include "disks.h"

class SysTray : public QObject
{
    Q_OBJECT
public:
    explicit SysTray(QObject *parent = NULL);
private:
    QSystemTrayIcon *disktray;
    QMenu *menu;
    Disks *man;
private slots:
    void generateContextMenu();
    void disktrayActivated(QSystemTrayIcon::ActivationReason reason);
    void handleDisktrayMessageClicked();
    void showMessage(QString title, QString message);
    void handleContextMenuAction();
    void handleDeviceError(QString path, QString error);
    void handleDeviceMediaChanged(QString path, bool media);
    void handleDeviceMountpointChanged(QString path, QString mountpoint);
    void openMountpoint(QString mountpoint);
    void handleFoundNewDevice(QString path);
    void handleShowHideDisktray();
};

#endif // SYSTRAY_H
