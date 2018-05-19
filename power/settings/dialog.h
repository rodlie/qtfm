/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>

class Dialog : public QDialog
{
    Q_OBJECT
public:
   explicit Dialog(QWidget *parent = NULL);
private:
    QComboBox *lidActionBattery;
    QComboBox *lidActionAC;
    QComboBox *criticalActionBattery;
    QSpinBox *lowBattery;
    QSpinBox *criticalBattery;
    QSpinBox *autoSleepBattery;
    QSpinBox *autoSleepAC;
private slots:
    void populate();
    void loadSettings();
    void updatePM();
    void setDefaultAction(QComboBox *box, int action);
    void setDefaultAction(QSpinBox *box, int action);
    void handleLidActionBattery(int index);
    void handleLidActionAC(int index);
    void handleCriticalAction(int index);
    void handleLowBattery(int value);
    void handleCriticalBattery(int value);
    void handleAutoSleepBattery(int value);
    void handleAutoSleepAC(int value);
};

#endif // DIALOG_H
