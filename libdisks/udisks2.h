/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef UDISKS2_H
#define UDISKS2_H

#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusInterface>
#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>

#define DBUS_SERVICE "org.freedesktop.UDisks2"
#define DBUS_PATH "/org/freedesktop/UDisks2"
#define DBUS_OBJMANAGER "org.freedesktop.DBus.ObjectManager"
#define DBUS_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define DBUS_DEVICE_ADDED "InterfacesAdded"
#define DBUS_DEVICE_REMOVED "InterfacesRemoved"

class uDisks2
{
public:
    static QString getDrivePath(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Block").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return QString(); }
        QDBusObjectPath drive = iface.property("Drive").value<QDBusObjectPath>();
        return drive.path();
    }
    static bool hasPartition(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Partition").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        if (iface.property("Number").toUInt()>=1) { return true; }
        return false;
    }
    static QString getFileSystem(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Block").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return QString(); }
        return iface.property("IdType").toString();
    }
    static bool isRemovable(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("Removable").toBool();
    }
    static bool isOptical(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        QStringList compat = iface.property("MediaCompatibility").toStringList();
        for (int i=0;i<compat.size();i++) {
            if (compat.at(i).startsWith("optical_")) { return true; }
        }
        return false;
    }
    static bool hasMedia(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("MediaAvailable").toBool();
    }
    static bool hasOpticalMedia(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        QString type = iface.property("Media").toString();
        if (type.startsWith("optical_")) { return true; }
        return false;
    }
    static bool canEject(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("Ejectable").toBool();
    }
    static bool opticalMediaIsBlank(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("OpticalBlank").toBool();
    }
    static int opticalDataTracks(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("OpticalNumDataTracks").toBool();
    }
    static int opticalAudioTracks(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        return iface.property("OpticalNumAudioTracks").toBool();
    }
    static QString getMountPoint(QString path)
    {
        QString mountpoint;
        QDBusMessage message = QDBusMessage::createMethodCall(DBUS_SERVICE, path, DBUS_PROPERTIES, "Get");
        QList<QVariant> args;
        args << QString("%1.Filesystem").arg(DBUS_SERVICE) << "MountPoints";
        message.setArguments(args);
        QDBusMessage reply = QDBusConnection::systemBus().call(message);
        QStringList mountpoints;
        QList<QByteArray> argList;
        foreach (QVariant arg, reply.arguments()) {
            if (!arg.value<QDBusVariant>().variant().isValid()) { continue; }
            arg.value<QDBusVariant>().variant().value<QDBusArgument>() >> argList;
        }
        foreach (QByteArray point, argList) { mountpoints.append(point); }
        mountpoint = mountpoints.join("");
        return mountpoint;
    }
    static QString getDeviceName(QString path)
    {
        QDBusInterface iface(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!iface.isValid()) { return QString(); }
        QString name = iface.property("Vendor").toString().simplified();
        if (!name.isEmpty()) { name.append(" "); }
        name.append(iface.property("Model").toString().simplified());
        return name;
    }
    static QString mountDevice(QString path)
    {
        QDBusInterface filesystem(DBUS_SERVICE, path, QString("%1.Filesystem").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!filesystem.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QVariantMap options;
        if (getFileSystem(path) == "vfat") { options.insert("options", "flush"); }
        QDBusReply<QString> mountpoint =  filesystem.call("Mount", options);
        return mountpoint.error().message();
    }
    static QString unmountDevice(QString path)
    {
        QDBusInterface filesystem(DBUS_SERVICE, path, QString("%1.Filesystem").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!filesystem.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QDBusMessage reply = filesystem.call("Unmount", QVariantMap());
        return reply.arguments().first().toString();
    }
    static QString ejectDevice(QString path)
    {
        QDBusInterface filesystem(DBUS_SERVICE, path, QString("%1.Drive").arg(DBUS_SERVICE), QDBusConnection::systemBus());
        if (!filesystem.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QDBusMessage reply = filesystem.call("Eject", QVariantMap());
        return reply.arguments().first().toString();
    }
    static QStringList getDevices()
    {
        QStringList result;
        QDBusMessage call = QDBusMessage::createMethodCall(DBUS_SERVICE, QString("%1/block_devices").arg(DBUS_PATH), DBUS_INTROSPECTABLE, "Introspect");
        QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);
        QList<QDBusObjectPath> devices;
        QXmlStreamReader xml(reply.value());
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name().toString() == "node" ) {
                QString name = xml.attributes().value("name").toString();
                if(!name.isEmpty()) { devices << QDBusObjectPath("/org/freedesktop/UDisks2/block_devices/" + name); }
            }
        }
        foreach (QDBusObjectPath device, devices) {
            result << device.path();
        }
        return result;
    }
};

#endif // UDISKS2_H
