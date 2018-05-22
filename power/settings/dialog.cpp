/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include "dialog.h"
#include <QVBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QTabWidget>
#include "common.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QMessageBox>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , lidActionBattery(0)
    , lidActionAC(0)
    , criticalActionBattery(0)
    , lowBattery(0)
    , criticalBattery(0)
    , autoSleepBattery(0)
    , autoSleepAC(0)
    , desktopSS(0)
    , desktopPM(0)
    , showNotifications(0)
    , showBatteryPercent(0)
    , showSystemTray(0)
{
    // setup dialog
    setAttribute(Qt::WA_QuitOnClose, true);
    setWindowTitle(tr("Power Settings"));
    setWindowIcon(QIcon::fromTheme(DEFAULT_BATTERY_ICON, QIcon(":/battery.png")));
    setMinimumSize(QSize(400,200));

    // setup widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *containerWidget = new QTabWidget(this);

    QWidget *wrapper = new QWidget(this);
    QHBoxLayout *wrapperLayout = new QHBoxLayout(wrapper);
    layout->addWidget(wrapper);

    QLabel *powerLabel = new QLabel(this);
    QIcon powerIcon = QIcon::fromTheme(DEFAULT_BATTERY_ICON, QIcon(":/battery.png"));
    powerLabel->setPixmap(powerIcon.pixmap(QSize(64, 64)));
    powerLabel->setMinimumSize(QSize(64, 64));
    powerLabel->setMaximumSize(QSize(64, 64));

    //layout->setSizeConstraint(QLayout::SetFixedSize); // lock dialog size
    wrapperLayout->addWidget(powerLabel);
    wrapperLayout->addWidget(containerWidget);

    QWidget *batteryContainer = new QWidget(this);
    QVBoxLayout *batteryContainerLayout = new QVBoxLayout(batteryContainer);
    batteryContainerLayout->setMargin(0);
    batteryContainerLayout->setSpacing(0);

    QWidget *lidActionBatteryContainer = new QWidget(this);
    QHBoxLayout *lidActionBatteryContainerLayout = new QHBoxLayout(lidActionBatteryContainer);
    lidActionBattery = new QComboBox(this);
    QLabel *lidActionBatteryLabel = new QLabel(this);

    lidActionBatteryLabel->setText(tr("Lid Action"));
    lidActionBatteryContainerLayout->addWidget(lidActionBatteryLabel);
    lidActionBatteryContainerLayout->addWidget(lidActionBattery);
    batteryContainerLayout->addWidget(lidActionBatteryContainer);

    QWidget *criticalActionBatteryContainer = new QWidget(this);
    QHBoxLayout *criticalActionBatteryContainerLayout = new QHBoxLayout(criticalActionBatteryContainer);
    criticalActionBattery = new QComboBox(this);
    QLabel *criticalActionBatteryLabel = new QLabel(this);

    criticalActionBatteryLabel->setText(tr("Critical Action"));
    criticalActionBatteryContainerLayout->addWidget(criticalActionBatteryLabel);
    criticalActionBatteryContainerLayout->addWidget(criticalActionBattery);
    batteryContainerLayout->addWidget(criticalActionBatteryContainer);

    QWidget *lowBatteryContainer = new QWidget(this);
    QHBoxLayout *lowBatteryContainerLayout = new QHBoxLayout(lowBatteryContainer);
    lowBattery = new QSpinBox(this);
    lowBattery->setMinimum(0);
    lowBattery->setMaximum(99);
    QLabel *lowBatteryLabel = new QLabel(this);

    lowBatteryLabel->setText(tr("Low battery (%)"));
    lowBatteryContainerLayout->addWidget(lowBatteryLabel);
    lowBatteryContainerLayout->addWidget(lowBattery);
    batteryContainerLayout->addWidget(lowBatteryContainer);

    QWidget *criticalBatteryContainer = new QWidget(this);
    QHBoxLayout *criticalBatteryContainerLayout = new QHBoxLayout(criticalBatteryContainer);
    criticalBattery = new QSpinBox(this);
    criticalBattery->setMinimum(0);
    criticalBattery->setMaximum(99);
    QLabel *criticalBatteryLabel = new QLabel(this);

    criticalBatteryLabel->setText(tr("Critical battery (%)"));
    criticalBatteryContainerLayout->addWidget(criticalBatteryLabel);
    criticalBatteryContainerLayout->addWidget(criticalBattery);
    batteryContainerLayout->addWidget(criticalBatteryContainer);

    QWidget *sleepBatteryContainer = new QWidget(this);
    QHBoxLayout *sleepBatteryContainerLayout = new QHBoxLayout(sleepBatteryContainer);
    autoSleepBattery = new QSpinBox(this);
    autoSleepBattery->setMinimum(0);
    autoSleepBattery->setMaximum(1000);
    QLabel *sleepBatteryLabel = new QLabel(this);

    sleepBatteryLabel->setText(tr("Auto sleep (min)"));
    sleepBatteryContainerLayout->addWidget(sleepBatteryLabel);
    sleepBatteryContainerLayout->addWidget(autoSleepBattery);
    batteryContainerLayout->addWidget(sleepBatteryContainer);

    batteryContainerLayout->addStretch();
    containerWidget->addTab(batteryContainer, tr("On Battery"));

    QWidget *acContainer = new QWidget(this);
    QVBoxLayout *acContainerLayout = new QVBoxLayout(acContainer);
    acContainerLayout->setMargin(0);
    acContainerLayout->setSpacing(0);

    QWidget *lidActionACContainer = new QWidget(this);
    QHBoxLayout *lidActionACContainerLayout = new QHBoxLayout(lidActionACContainer);
    lidActionAC = new QComboBox(this);
    QLabel *lidActionACLabel = new QLabel(this);

    lidActionACLabel->setText(tr("Lid Action"));
    lidActionACContainerLayout->addWidget(lidActionACLabel);
    lidActionACContainerLayout->addWidget(lidActionAC);
    acContainerLayout->addWidget(lidActionACContainer);

    QWidget *sleepACContainer = new QWidget(this);
    QHBoxLayout *sleepACContainerLayout = new QHBoxLayout(sleepACContainer);
    autoSleepAC = new QSpinBox(this);
    autoSleepAC->setMinimum(0);
    autoSleepAC->setMaximum(1000);
    QLabel *sleepACLabel = new QLabel(this);

    sleepACLabel->setText(tr("Auto sleep (min)"));
    sleepACContainerLayout->addWidget(sleepACLabel);
    sleepACContainerLayout->addWidget(autoSleepAC);
    acContainerLayout->addWidget(sleepACContainer);

    acContainerLayout->addStretch();
    containerWidget->addTab(acContainer, tr("On AC"));

    QWidget *desktopContainer = new QWidget(this);
    QVBoxLayout *desktopContainerLayout = new QVBoxLayout(desktopContainer);

    desktopSS = new QCheckBox(this);
    desktopSS->setText("org.freedesktop.ScreenSaver");
    desktopPM = new QCheckBox(this);
    desktopPM->setText("org.freedesktop.PowerManagement");

    desktopContainerLayout->addWidget(desktopSS);
    desktopContainerLayout->addWidget(desktopPM);

    desktopContainerLayout->addStretch();
    containerWidget->addTab(desktopContainer, tr("Desktop Integration"));

    QWidget *extraContainer = new QWidget(this);
    QHBoxLayout *extraContainerLayout = new QHBoxLayout(extraContainer);

    showSystemTray  = new QCheckBox(this);
    showSystemTray->setText(tr("System tray"));
    extraContainerLayout->addWidget(showSystemTray);

    showNotifications = new QCheckBox(this);
    showNotifications->setText(tr("Notifications"));
    extraContainerLayout->addWidget(showNotifications);

    showBatteryPercent = new QCheckBox(this);
    showBatteryPercent->setText(tr("Battery percent"));
    extraContainerLayout->addWidget(showBatteryPercent);

    layout->addWidget(extraContainer);

    populate(); // populate boxes
    loadSettings(); // load settings

    // connect varius widgets
    connect(lidActionBattery, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLidActionBattery(int)));
    connect(lidActionAC, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLidActionAC(int)));
    connect(criticalActionBattery, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCriticalAction(int)));
    connect(lowBattery, SIGNAL(valueChanged(int)), this, SLOT(handleLowBattery(int)));
    connect(criticalBattery, SIGNAL(valueChanged(int)), this, SLOT(handleCriticalBattery(int)));
    connect(autoSleepBattery, SIGNAL(valueChanged(int)), this, SLOT(handleAutoSleepBattery(int)));
    connect(autoSleepAC, SIGNAL(valueChanged(int)), this, SLOT(handleAutoSleepAC(int)));
    connect(desktopSS, SIGNAL(toggled(bool)), this, SLOT(handleDesktopSS(bool)));
    connect(desktopPM, SIGNAL(toggled(bool)), this, SLOT(handleDesktopPM(bool)));
    connect(showNotifications, SIGNAL(toggled(bool)), this, SLOT(handleShowNotifications(bool)));
    connect(showBatteryPercent, SIGNAL(toggled(bool)), this, SLOT(handleShowBatteryPercent(bool)));
    connect(showSystemTray, SIGNAL(toggled(bool)), this, SLOT(handleShowSystemTray(bool)));
}

// populate widgets with default values
void Dialog::populate()
{
    lidActionBattery->clear();
    lidActionBattery->addItem(tr("None"), lidNone);
    lidActionBattery->addItem(tr("Lock Screen"), lidLock);
    lidActionBattery->addItem(tr("Sleep"), lidSleep);
    lidActionBattery->addItem(tr("Hibernate"), lidHibernate);

    lidActionAC->clear();
    lidActionAC->addItem(tr("None"), lidNone);
    lidActionAC->addItem(tr("Lock Screen"), lidLock);
    lidActionAC->addItem(tr("Sleep"), lidSleep);
    lidActionAC->addItem(tr("Hibernate"), lidHibernate);

    criticalActionBattery->clear();
    criticalActionBattery->addItem(tr("None"), criticalNone);
    criticalActionBattery->addItem(tr("Hibernate"), criticalHibernate);
    criticalActionBattery->addItem(tr("Shutdown"), criticalShutdown);
}

// load settings and set as default in widgets
void Dialog::loadSettings()
{
    int defaultAutoSleepBattery = AUTO_SLEEP_BATTERY;
    if (Common::validPowerSettings("autoSleepBattery")) {
        defaultAutoSleepBattery = Common::loadPowerSettings("autoSleepBattery").toInt();
    }
    setDefaultAction(autoSleepBattery, defaultAutoSleepBattery);

    int defaultAutoSleepAC = 0; // don't add default on AC, not all (normal) machines support suspend.
    if (Common::validPowerSettings("autoSleepAC")) {
        defaultAutoSleepAC = Common::loadPowerSettings("autoSleepAC").toInt();
    }
    setDefaultAction(autoSleepAC, defaultAutoSleepAC);

    int defaultLowBattery = LOW_BATTERY;
    if (Common::validPowerSettings("lowBattery")) {
        defaultLowBattery = Common::loadPowerSettings("lowBattery").toInt();
    }
    setDefaultAction(lowBattery, defaultLowBattery);

    int defaultCriticalBattery = CRITICAL_BATTERY;
    if (Common::validPowerSettings("criticalBattery")) {
        defaultCriticalBattery = Common::loadPowerSettings("criticalBattery").toInt();
    }
    setDefaultAction(criticalBattery, defaultCriticalBattery);

    int defaultLidActionBattery = LID_BATTERY_DEFAULT;
    if (Common::validPowerSettings("lidBattery")) {
        defaultLidActionBattery = Common::loadPowerSettings("lidBattery").toInt();
    }
    setDefaultAction(lidActionBattery, defaultLidActionBattery);

    int defaultLidActionAC = LID_AC_DEFAULT;
    if (Common::validPowerSettings("lidAC")) {
        defaultLidActionAC = Common::loadPowerSettings("lidAC").toInt();
    }
    setDefaultAction(lidActionAC, defaultLidActionAC);

    int defaultCriticalAction = CRITICAL_DEFAULT;
    if (Common::validPowerSettings("criticalAction")) {
        defaultCriticalAction = Common::loadPowerSettings("criticalAction").toInt();
    }
    setDefaultAction(criticalActionBattery, defaultCriticalAction);

    bool defaultDesktopSS = true;
    if (Common::validPowerSettings("desktop_ss")) {
        defaultDesktopSS = Common::loadPowerSettings("desktop_ss").toBool();
    }
    desktopSS->setChecked(defaultDesktopSS);

    bool defaultDesktopPM = true;
    if (Common::validPowerSettings("desktop_pm")) {
        defaultDesktopPM = Common::loadPowerSettings("desktop_pm").toBool();
    }
    desktopPM->setChecked(defaultDesktopPM);

    bool defaultShowNotifications = true;
    if (Common::validPowerSettings("tray_notify")) {
        defaultShowNotifications = Common::loadPowerSettings("tray_notify").toBool();
    }
    showNotifications->setChecked(defaultShowNotifications);

    bool defaultShowBatteryPercent = true;
    if (Common::validPowerSettings("show_battery_percent")) {
        defaultShowBatteryPercent = Common::loadPowerSettings("show_battery_percent").toBool();
    }
    showBatteryPercent->setChecked(defaultShowBatteryPercent);

    bool defaultShowTray = true;
    if (Common::validPowerSettings("show_tray")) {
        defaultShowTray = Common::loadPowerSettings("show_tray").toBool();
    }
    showSystemTray->setChecked(defaultShowTray);
}

// tell power manager to update settings
void Dialog::updatePM()
{
    QDBusInterface iface(PM_SERVICE, PM_PATH, PM_SERVICE, QDBusConnection::sessionBus());
    if (!iface.isValid()) { return; }
    iface.call("refresh");
}

// set default action in combobox
void Dialog::setDefaultAction(QComboBox *box, int action)
{
    for (int i=0;i<box->count();i++) {
        if (box->itemData(i).toInt() == action) {
            box->setCurrentIndex(i);
            return;
        }
    }
}

// set default value in spinbox
void Dialog::setDefaultAction(QSpinBox *box, int action)
{
    box->setValue(action);
}

// save current value and update power manager
void Dialog::handleLidActionBattery(int index)
{
    Common::savePowerSettings("lidBattery", index);
    updatePM();
}

void Dialog::handleLidActionAC(int index)
{
    Common::savePowerSettings("lidAC", index);
    updatePM();
}

void Dialog::handleCriticalAction(int index)
{
    Common::savePowerSettings("criticalAction", index);
    updatePM();
}

void Dialog::handleLowBattery(int value)
{
    Common::savePowerSettings("lowBattery", value);
    updatePM();
}

void Dialog::handleCriticalBattery(int value)
{
    Common::savePowerSettings("criticalBattery", value);
    updatePM();
}

void Dialog::handleAutoSleepBattery(int value)
{
    Common::savePowerSettings("autoSleepBattery", value);
    updatePM();
}

void Dialog::handleAutoSleepAC(int value)
{
    Common::savePowerSettings("autoSleepAC", value);
    updatePM();
}

void Dialog::handleDesktopSS(bool triggered)
{
    Common::savePowerSettings("desktop_ss", triggered);
    updatePM();
    QMessageBox::information(this, tr("Restart required"), tr("You must restart the power daemon to apply this setting"));
}

void Dialog::handleDesktopPM(bool triggered)
{
    Common::savePowerSettings("desktop_pm", triggered);
    updatePM();
    QMessageBox::information(this, tr("Restart required"), tr("You must restart the power daemon to apply this setting"));
}

void Dialog::handleShowNotifications(bool triggered)
{
    Common::savePowerSettings("tray_notify", triggered);
    updatePM();
}

void Dialog::handleShowBatteryPercent(bool triggered)
{
    Common::savePowerSettings("show_battery_percent", triggered);
    updatePM();
}

void Dialog::handleShowSystemTray(bool triggered)
{
    Common::savePowerSettings("show_tray", triggered);
    updatePM();
}
