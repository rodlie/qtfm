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

#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H


#include <QtGui>
#include <QLabel>
#include <QDialog>
#include <QToolButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include "mymodel.h"

/**
 * @class PropertiesDialog
 * @brief The PropertiesDialog class
 * @author Wittefella
 * @date unknown
 */
class PropertiesDialog: public QDialog {
  Q_OBJECT
public:
  PropertiesDialog(QStringList, myModel *);
public slots:
  void accept();
  void reject();
  void update();
  void checkboxesChanged();
  void changeIcon();
  void numericChanged(QString);
  void finished();
signals:
  void finishedSignal();
  void updateSignal();
  void propertiesUpdated();
private:
  void folderProperties(QStringList paths);
  void recurseProperties(QString path);

  QToolButton *iconButton;
  QLabel *path;
  QLabel *sizeInfo;
  QLabel *containsInfo;
  QLabel *modifiedInfo;
  QLabel *driveInfo;

  QCheckBox * ownerRead;
  QCheckBox * ownerWrite;
  QCheckBox * ownerExec;

  QCheckBox * groupRead;
  QCheckBox * groupWrite;
  QCheckBox * groupExec;

  QCheckBox * otherRead;
  QCheckBox * otherWrite;
  QCheckBox * otherExec;

  QLineEdit * permissionsNumeric;

  QDialogButtonBox *buttons;
  QFuture<void> thread;

  QStringList fileList;
  QString pathName;
  QString permString;
  bool iconChanged;
  myModel * model;
  QHash<QString,QIcon> *folderIcons;
  QHash<QString,QIcon> *fileIcons;

  int type;           // 1=folder, 2=file, 3=multiple
  qint64 files;
  qint64 folders;
  qint64 totalSize;
};

#endif // PROPERTIESDLG_H
