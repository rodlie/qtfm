/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "systray.h"
#include <QIcon>
#include <QProcess>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QDebug>

SysTray::SysTray(QObject *parent)
    : QObject(parent)
    , disktray(0)
    , menu(0)
    , man(0)
{
    menu = new QMenu();

    disktray = new QSystemTrayIcon(QIcon::fromTheme("drive-removable-media"), this);
    connect(disktray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(disktrayActivated(QSystemTrayIcon::ActivationReason)));
    connect(disktray, SIGNAL(messageClicked()), this, SLOT(handleDisktrayMessageClicked()));

    man = new Disks(this);
    connect(man, SIGNAL(updatedDevices()), this, SLOT(generateContextMenu()));
    connect(man, SIGNAL(deviceErrorMessage(QString,QString)), this, SLOT(handleDeviceError(QString,QString)));
    connect(man, SIGNAL(mediaChanged(QString,bool)), this, SLOT(handleDeviceMediaChanged(QString,bool)));
    connect(man, SIGNAL(mountpointChanged(QString,QString)), this, SLOT(handleDeviceMountpointChanged(QString,QString)));
    connect(man, SIGNAL(foundNewDevice(QString)), this, SLOT(handleFoundNewDevice(QString)));
    QTimer::singleShot(10000, this, SLOT(generateContextMenu()));
}

void SysTray::generateContextMenu()
{
    for(int i=0;i<menu->actions().size();i++) {
        menu->actions().at(i)->disconnect();
        delete menu->actions().at(i);
    }
    menu->clear();

    QMapIterator<QString, Device*> device(man->devices);
    while (device.hasNext()) {
        device.next();
        if ((device.value()->isOptical && !device.value()->hasMedia) || (!device.value()->isOptical && !device.value()->isRemovable) || (!device.value()->isOptical && !device.value()->hasPartition)) {
            continue;
        }

        QAction *deviceAction = new QAction(this);
        deviceAction->setData(device.key());
        deviceAction->setText(QString("%1 (%2)").arg(device.value()->name).arg(device.value()->dev));

        connect(deviceAction, SIGNAL(triggered(bool)), this, SLOT(handleContextMenuAction()));
        menu->addAction(deviceAction);

        if (device.value()->mountpoint.isEmpty()) {
            deviceAction->setIcon(QIcon::fromTheme(device.value()->isOptical?"drive-optical":"drive-removable-media"));
            bool hasAudio = device.value()->opticalAudioTracks>0?true:false;
            bool hasData = device.value()->opticalDataTracks>0?true:false;
            if (device.value()->isBlankDisc||(hasAudio&&!hasData)) { deviceAction->setIcon(QIcon::fromTheme("media-eject")); }
        } else { deviceAction->setIcon(QIcon::fromTheme("media-eject")); }
    }

    //qDebug() << menu->actions();
    handleShowHideDisktray();
}

void SysTray::disktrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
    case QSystemTrayIcon::Context:
    case QSystemTrayIcon::Trigger:
        if (menu->actions().size()>0) { menu->popup(QCursor::pos()); }
    default:;
    }
}

void SysTray::handleDisktrayMessageClicked()
{
    handleShowHideDisktray();
}


void SysTray::showMessage(QString title, QString message)
{
    if (!disktray->isSystemTrayAvailable()) { return; }
    if (!disktray->isVisible()) { disktray->show(); }
    disktray->showMessage(title, message);
    QTimer::singleShot(10000, this, SLOT(handleShowHideDisktray()));
}

void SysTray::handleContextMenuAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action==NULL) { return; }
    QString path = action->data().toString();
    if (path.isEmpty()) { return; }
    if (!man->devices.contains(path)) { return; }

    if (man->devices[path]->mountpoint.isEmpty()) { // mount
        if (man->devices[path]->isOptical && (man->devices[path]->isBlankDisc || man->devices[path]->opticalDataTracks==0)) { man->devices[path]->eject(); }
        else { man->devices[path]->mount(); }
    } else { // unmount
        man->devices[path]->unmount();
    }
    generateContextMenu();
}

void SysTray::handleDeviceError(QString path, QString error)
{
    if (!man->devices.contains(path)) { return; }
    showMessage(QObject::tr("Error for device %1").arg(man->devices[path]->name), error);
}

void SysTray::handleDeviceMediaChanged(QString path, bool media)
{
    if (!man->devices.contains(path)) { return; }
    generateContextMenu();
    if (man->devices[path]->isOptical && media) {
        bool isData = man->devices[path]->opticalDataTracks>0?true:false;
        bool isAudio = man->devices[path]->opticalAudioTracks>0?true:false;
        QString opticalType;
        if (isData&&isAudio) { opticalType = QObject::tr("data+audio"); }
        else if (isData) { opticalType = QObject::tr("data"); }
        else if (isAudio) { opticalType = QObject::tr("audio"); }
        showMessage(QObject::tr("%1 has media").arg(man->devices[path]->name), QObject::tr("Detected %1 media in %2").arg(opticalType).arg(man->devices[path]->name));
    }
}

void SysTray::handleDeviceMountpointChanged(QString path, QString mountpoint)
{
    if (!man->devices.contains(path)) { return; }
    generateContextMenu();
    if (!man->devices[path]->isRemovable) { return; }
    if (mountpoint.isEmpty()) {
        if (!man->devices[path]->isOptical) {
            showMessage(QObject::tr("%1 removed").arg(man->devices[path]->name), QObject::tr("It's now safe to remove %1 from your computer.").arg(man->devices[path]->name));
        }
    } else { openMountpoint(mountpoint); }
}

void SysTray::openMountpoint(QString mountpoint)
{
    if (mountpoint.isEmpty()) { return; }
    QProcess::startDetached(QString("qtfm %1").arg(mountpoint));
}

void SysTray::handleFoundNewDevice(QString path)
{
    if (!man->devices.contains(path)) { return; }
    showMessage(QString("Found %1").arg(man->devices[path]->name), QString("Found a new device (%1)").arg(man->devices[path]->dev));
}

void SysTray::handleShowHideDisktray()
{
    if (menu->actions().size()==0) {
        if (disktray->isVisible()) { disktray->hide(); }
    } else {
        if (!disktray->isVisible() && disktray->isSystemTrayAvailable()) { disktray->show(); }
    }
}
