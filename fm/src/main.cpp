/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2012, 2013 Michal Rost
* Copyright (C) 2010, 2011, 2012 Wittfella
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
* Contact e-mail: rost.michal@gmail.com, wittfella@qtfm.org
*
****************************************************************************/

#include <QApplication>
#include "mainwindow.h"

/**
 * @brief main function
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @return 0/1
 */
int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  // Connect to daemon if available, otherwise create new instance
  if (app.arguments().count() == 1) {
    QLocalServer server;
    if (!server.listen(APP)) {
      QLocalSocket client;
      client.connectToServer(APP);
      client.waitForConnected(1000);
      if (client.state() != QLocalSocket::ConnectedState) {
        QFile::remove(QDir::tempPath() + QString("/%1").arg(APP));
      } else {
        client.close();
        return 0;
      }
    }
    server.close();
  }

  // Initialize resources
 // Q_INIT_RESOURCE(resources);

  // Set application info
  QString appName = APP;
  appName.append("6"); // don't use config from QtFM 5 or lower
  app.setOrganizationName(appName);
  app.setApplicationName(appName);
  app.setOrganizationDomain("org.dracolinux");

  // Translate application
  /*QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  QTranslator qtfmTranslator;
  qtfmTranslator.load("/usr/share/qtfm/qtfm_" + QLocale::system().name());
  app.installTranslator(&qtfmTranslator);*/

  // Create main window and execute application
  MainWindow mainWin;
  return app.exec();
}
