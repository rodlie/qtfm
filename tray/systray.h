/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
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
 */

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QHash>
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
    QHash<QString, QIcon> appsCache;

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
    void generateAppsCache();
};

#endif // SYSTRAY_H
