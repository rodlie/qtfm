#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDebug>

#define FM_MAJOR 6

class Common
{
public:
    static QString configDir()
    {
        QString dir = QString("%1/.config/%2%3").arg(QDir::homePath()).arg(APP).arg(FM_MAJOR);
        qDebug() << "common config dir" << dir;
        if (!QFile::exists(dir)) {
            QDir makedir(dir);
            if (!makedir.mkpath(dir)) { dir.clear(); }
        }
        return dir;
    }
};

#endif // COMMON_H
