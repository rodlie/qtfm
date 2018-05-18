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


#include "icondlg.h"
#if QT_VERSION >= 0x050000
  #include <QtConcurrent/QtConcurrent>
#else
#endif

#include "common.h"

//---------------------------------------------------------------------------
icondlg::icondlg()
{
    setWindowTitle(tr("Select icon"));

    iconList = new QListWidget;
    iconList->setIconSize(QSize(24,24));
    iconList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    buttons = new QDialogButtonBox;
    buttons->setStandardButtons(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(iconList);
    layout->addWidget(buttons);
    setLayout(layout);

    for (int i=0;i<Common::iconLocations().size();++i) {
        QSettings inherits(Common::iconLocations().at(i) + "/" + QIcon::themeName() + "/index.theme",QSettings::IniFormat,this);
        foreach(QString theme, inherits.value("Icon Theme/Inherits").toStringList()) {
            themes.prepend(theme);
            themes.append(QIcon::themeName());
        }
    }

    thread.setFuture(QtConcurrent::run(this,&icondlg::scanTheme));
    connect(&thread,SIGNAL(finished()),this,SLOT(loadIcons()));
}

//---------------------------------------------------------------------------
void icondlg::scanTheme()
{
    foreach(QString theme, themes) {
        //qDebug() << theme;
        for (int i=0;i<Common::iconLocations().size();++i) {
            QDirIterator it(Common::iconLocations().at(i) + "/" + theme, QStringList("*.png"), QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                fileNames.append(QFileInfo(it.fileName()).baseName());
            }
        }
    }

    fileNames.removeDuplicates();
    fileNames.sort();
}

//---------------------------------------------------------------------------
void icondlg::loadIcons()
{
    int counter = 0;

    foreach(QString name, fileNames)
    {
        new QListWidgetItem(QIcon::fromTheme(name),name,iconList);
        fileNames.removeOne(name);
        counter++;
        if(counter == 20)
        {
            QTimer::singleShot(50,this,SLOT(loadIcons()));
            return;
        }
    }
}

//---------------------------------------------------------------------------
void icondlg::accept()
{
    result = iconList->currentItem()->text();
    this->done(1);
}
