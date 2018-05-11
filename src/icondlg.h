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

#ifndef ICONDLG_H
#define ICONDLG_H

#include <QtGui>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QCompleter>
#include <QDialogButtonBox>

class icondlg : public QDialog
{

Q_OBJECT

public:
    icondlg();
    QString result;

public slots:
    void scanTheme();
    void loadIcons();
    void accept();

private:
    QListWidget *iconList;
    QDialogButtonBox *buttons;
    QVBoxLayout *layout;
    QStringList fileNames;
    QStringList themes;
    QFutureWatcher<void> thread;
};

#endif // ICONDLG_H
