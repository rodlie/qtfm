#ifndef COMMON_H
#define COMMON_H

#include <QSettings>
#include <QVariant>

enum lidAction
{
    lidNone,
    lidLock,
    lidSleep,
    lidHibernate
};

enum criticalAction
{
    criticalNone,
    criticalHibernate,
    criticalShutdown
};

#define LID_BATTERY_DEFAULT lidSleep
#define LID_AC_DEFAULT lidLock
#define CRITICAL_DEFAULT criticalNone

#define LOW_BATTERY 15
#define CRITICAL_BATTERY 10
#define AUTO_SLEEP_BATTERY 15
#define DEFAULT_BATTERY_ICON "battery"
#define DEFAULT_BATTERY_ICON_CRIT "battery-caution"
#define DEFAULT_BATTERY_ICON_CRIT_AC "battery-caution-charging"
#define DEFAULT_BATTERY_ICON_LOW "battery-low"
#define DEFAULT_BATTERY_ICON_LOW_AC "battery-low-charging"
#define DEFAULT_BATTERY_ICON_GOOD "battery-good"
#define DEFAULT_BATTERY_ICON_GOOD_AC "battery-good-charging"
#define DEFAULT_BATTERY_ICON_FULL "battery-full"
#define DEFAULT_BATTERY_ICON_FULL_AC "battery-full-charging"
#define DEFAULT_BATTERY_ICON_CHARGED "battery-full-charged"
#define DEFAULT_BATTERY_EMPTY "battery-empty"
#define DEFAULT_BATTERY_MISSING "battery-missing"

#define PM_SERVICE "org.freedesktop.PowerManagement"
#define PM_PATH "/PowerManagement"

#ifndef CONFIG_DIR
#define CONFIG_DIR "qtfm"
#endif

class Common
{
public:
    static void savePowerSettings(QString type, QVariant value)
    {
        QSettings settings(CONFIG_DIR, "power");
        settings.setValue(type, value);
    }
    static QVariant loadPowerSettings(QString type)
    {
        QSettings settings(CONFIG_DIR, "power");
        return settings.value(type);
    }
    static bool validPowerSettings(QString type)
    {
        QSettings settings(CONFIG_DIR, "power");
        return settings.value(type).isValid();
    }
};

#endif // COMMON_H
