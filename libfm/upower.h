/*
#
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef UPOWER_H
#define UPOWER_H

#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusInterface>
#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>

#define DBUS_SERVICE "org.freedesktop.UPower"
#define DBUS_PATH "/org/freedesktop/UPower"
#define DBUS_OBJMANAGER "org.freedesktop.DBus.ObjectManager"
#define DBUS_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define DBUS_DEVICE_ADDED "DeviceAdded"
#define DBUS_DEVICE_REMOVED "DeviceRemoved"

#define CKIT_SERVICE "org.freedesktop.ConsoleKit"
#define CKIT_PATH "/org/freedesktop/ConsoleKit/Manager"
#define CKIT_MANAGER "org.freedesktop.ConsoleKit.Manager"

class UPower
{
public:
    static bool canPowerOff()
    {
        QDBusInterface iface(CKIT_SERVICE, CKIT_PATH, CKIT_MANAGER, QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        QDBusMessage reply = iface.call("CanPowerOff");
        return reply.arguments().first().toBool();
    }
    static QString poweroff()
    {
        QDBusInterface iface(CKIT_SERVICE, CKIT_PATH, CKIT_MANAGER, QDBusConnection::systemBus());
        if (!iface.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QDBusMessage reply = iface.call("PowerOff");
        return reply.arguments().first().toString();
    }
    static bool canSuspend()
    {
        QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_SERVICE, QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        QDBusMessage reply = iface.call("CanSuspend");
        return reply.arguments().first().toBool();
    }
    static QString suspend()
    {
        QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_SERVICE, QDBusConnection::systemBus());
        if (!iface.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QDBusMessage reply = iface.call("Suspend");
        return reply.arguments().first().toString();
    }
    static bool canHibernate()
    {
        QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_SERVICE, QDBusConnection::systemBus());
        if (!iface.isValid()) { return false; }
        QDBusMessage reply = iface.call("CanHibernate");
        return reply.arguments().first().toBool();
    }
    static QString hibernate()
    {
        QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_SERVICE, QDBusConnection::systemBus());
        if (!iface.isValid()) { return QObject::tr("Failed D-Bus connection."); }
        QDBusMessage reply = iface.call("Hibernate");
        return reply.arguments().first().toString();
    }
    static QStringList getDevices()
    {
        QStringList result;
        QDBusMessage call = QDBusMessage::createMethodCall(DBUS_SERVICE, QString("%1/devices").arg(DBUS_PATH), DBUS_INTROSPECTABLE, "Introspect");
        QDBusPendingReply<QString> reply = QDBusConnection::systemBus().call(call);
        QList<QDBusObjectPath> devices;
        QXmlStreamReader xml(reply.value());
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name().toString() == "node" ) {
                QString name = xml.attributes().value("name").toString();
                if(!name.isEmpty()) { devices << QDBusObjectPath("/org/freedesktop/UPower/devices/" + name); }
            }
        }
        foreach (QDBusObjectPath device, devices) {
            result << device.path();
        }
        return result;
    }
};

#endif // POWER_H
