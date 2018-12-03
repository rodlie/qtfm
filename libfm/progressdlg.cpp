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

#include <QtGui>
#include <QPushButton>
#include <QGridLayout>
#include "progressdlg.h"

//---------------------------------------------------------------------------
myProgressDialog::myProgressDialog(QString title)
{
   setWindowTitle(title);

   filename = new QLabel(tr("Initializing..."));
   bar = new QProgressBar();
   button = new QPushButton("Cancel");

   //transfer info label
   transferInfo = new QLabel();
   transferInfo->setText(QString("<p><br></p>"));

   //remaining time timer
   remainingTimer = new QTime();
   oldSeconds = 0;

   runningTotal = 0;
   bar->setRange(0,100);

   filename->setFixedWidth(300);
   bar->setFixedWidth(300);
   bar->setAlignment(Qt::AlignHCenter);
   connect(button, SIGNAL(clicked()), this, SLOT(accept()));

   QGridLayout *layout = new QGridLayout();
   layout->addWidget(filename,1,1,1,4);
   layout->addWidget(bar,2,1,1,4);
   layout->addWidget(transferInfo,3,1,1,1,Qt::AlignBottom);
   layout->addWidget(button,3,4,1,1,Qt::AlignBottom);
   setLayout(layout);

   //start the timer
   remainingTimer->start();

   connect(button, SIGNAL(released()), this, SLOT(clearFilename()));

   QTimer::singleShot(1000,this,SLOT(setShowing()));
}

//---------------------------------------------------------------------------
void myProgressDialog::setShowing()
{
    if(bar->value() < 70) open();
    return;
}

//---------------------------------------------------------------------------
void myProgressDialog::update(qint64 bytes, qint64 total, QString name)
{
    //set file name
    realFilename = name;
    filename->setText(name.split("/").takeLast());
    filename->setToolTip(realFilename);

    //refresh the bar
    runningTotal += bytes;
    bar->setValue(runningTotal * 100 / total);

    //transfer info
    int currentSeconds = remainingTimer->elapsed() / 1000;  //convert to seconds

    if (currentSeconds != oldSeconds)                       //enter here every second
    {
        float cumulativeTransferRate = runningTotal / currentSeconds;
        float cumulativeTransferRateMB = cumulativeTransferRate / 1000000; //convert to megabytes
        int currentSecondsRemaining = (total - runningTotal) / cumulativeTransferRate;

        QString formattedTime;

        if (currentSecondsRemaining < 60) formattedTime = QString("%1 seconds").arg(currentSecondsRemaining);
        else formattedTime = QString("%1 min %2 sec").arg(currentSecondsRemaining / 60).arg(currentSecondsRemaining % 60);

        transferInfo->setText(QString(tr("<p>Transfer rate: %2 MB/s<br>Time remaining: %3</p>"))
                                   .arg(cumulativeTransferRateMB, 0, 'f', 1).arg(formattedTime));

        oldSeconds = currentSeconds;
    }

    return;
}

QString myProgressDialog::getFilename()
{
    return realFilename;
}

void myProgressDialog::clearFilename()
{
    realFilename.clear();
    setResult(0);
}


