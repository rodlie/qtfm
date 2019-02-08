/*
 * This file is part of QtFM <https://qtfm.eu>
 *
 * Copyright (C) 2013-2019 QtFM developers (see AUTHORS)
 * Copyright (C) 2010-2012 Wittfella <wittfella@qtfm.org>
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
 */

#include "dialog.h"
#include "desktopfile.h"
#include "mimeutils.h"
#include "common.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc>1) {
        QString fileName = argv[1];
        if (QFile::exists(fileName)) {
            if (fileName.endsWith(QString(".desktop"))) { // open app
                DesktopFile desktop(fileName);
                if (!desktop.getExec().isEmpty()) {
                    QProcess::startDetached(desktop.getExec()
                                            .split(QString(" "))
                                            .takeFirst());
                    return 0;
                }
            } else { // open file
                MimeUtils mimeUtils;
                QString mime = mimeUtils.getMimeType(fileName);
                if (!mime.isEmpty()) {
                    QStringList appNames = mimeUtils.getDefault(mime);
                    DesktopFile desktop(Common::findApplication(qApp->applicationFilePath(),
                                                                appNames.at(0)));
                    if (!desktop.getExec().isEmpty()) {
                        QFileInfo fileInfo(fileName);
                        mimeUtils.openInApp(desktop.getExec(),
                                            fileInfo,
                                            QString());
                        return 0;
                    }
                }
            }
        }
        return 1;
    } else {
        Dialog d;
        d.show();
        return a.exec();
    }
}
