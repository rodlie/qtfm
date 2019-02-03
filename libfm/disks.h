/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef DISKS_H
#define DISKS_H

#include <QObject>
#include <QMap>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QTimer>

class Device : public QObject
{
    Q_OBJECT

public:
    explicit Device(const QString block, QObject *parent = Q_NULLPTR);
    QString name;
    QString path;
    QString dev;
    QString drive;
    QString mountpoint;
    QString filesystem;
    bool isOptical;
    bool isRemovable;
    bool hasMedia;
    int opticalDataTracks;
    int opticalAudioTracks;
    bool isBlankDisc;
    bool hasPartition;

private:
    QDBusInterface *dbus;

signals:
    void mediaChanged(QString devicePath, bool mediaPresent);
    void mountpointChanged(QString devicePath, QString deviceMountpoint);
    void nameChanged(QString devicePath, QString deviceName);
    void errorMessage(QString devicePath, QString deviceError);

public slots:
    void mount();
    void unmount();
    void eject();

private slots:
    void updateDeviceProperties();
    void handlePropertiesChanged(const QString &interfaceType, const QMap<QString, QVariant> &changedProperties);
};

class Disks : public QObject
{
    Q_OBJECT

public:
    explicit Disks(QObject *parent = Q_NULLPTR);
    QMap<QString, Device*> devices;

private:
    QDBusInterface *dbus;
    QTimer timer;

signals:
    void updatedDevices();
    void mediaChanged(QString path, bool media);
    void mountpointChanged(QString path, QString mountpoint);
    void deviceErrorMessage(QString path, QString error);
    void foundNewDevice(QString path);
    void removedDevice(QString path);

private slots:
    void setupDBus();
    void scanDevices();
    void deviceAdded(const QDBusObjectPath &obj);
    void deviceRemoved(const QDBusObjectPath &obj);
    void handleDeviceMediaChanged(QString devicePath, bool mediaPresent);
    void handleDeviceMountpointChanged(QString devicePath, QString deviceMountpoint);
    void handleDeviceErrorMessage(QString devicePath, QString deviceError);
    void checkUDisks();
};

#endif // DISKS_H
