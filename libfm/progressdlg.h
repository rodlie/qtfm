/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
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
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/

#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H


#include <QtGui>
#include <QDialog>
#include <QLabel>
#include <QString>
#include <QProgressBar>

class myProgressDialog : public QDialog
{
    Q_OBJECT

public:
    myProgressDialog(QString title);

public slots:
    void setShowing();
    void update(qint64 bytes, qint64 total, QString name);
    QString getFilename();

private:
    QString realFilename;
    QLabel *filename;
    QLabel *transferInfo;
    QProgressBar *bar;
    QPushButton *button;

    QTime  *remainingTimer;

    qint64 runningTotal;
    int oldSeconds;

private slots:
    void clearFilename();
};


#endif // PROGRESSDLG_H
