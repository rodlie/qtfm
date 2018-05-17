#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QDirIterator>
#include <QRegExp>
#include <QTextStream>

#define FM_MAJOR 6

class Common
{
public:
    static QString configDir()
    {
        QString dir = QString("%1/.config/%2%3").arg(QDir::homePath()).arg(APP).arg(FM_MAJOR);
        if (!QFile::exists(dir)) {
            QDir makedir(dir);
            if (!makedir.mkpath(dir)) { dir.clear(); }
        }
        return dir;
    }
    static QStringList iconLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/icons").arg(QDir::homePath());
        result << QString("%1/../share/icons").arg(qApp->applicationFilePath());
        result << "/usr/share/icons" << "/usr/local/share/icons";
        return result;
    }
    static QStringList pixmapLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/pixmaps").arg(QDir::homePath());
        result << QString("%1/../share/pixmaps").arg(qApp->applicationFilePath());
        result << "/usr/share/pixmaps" << "/usr/local/share/pixmaps";
        return result;
    }
    static QStringList applicationLocations()
    {
        QStringList result;
        result << QString("%1/.local/share/applications").arg(QDir::homePath());
        result << QString("%1/../share/applications").arg(qApp->applicationFilePath());
        result << "/usr/share/applications" << "/usr/local/share/applications";
        return result;
    }
    static QString getDesktopIcon(QString desktop)
    {
        QString result;
        if (desktop.isEmpty()) { return result; }
        QFile file(desktop);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return result; }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().isEmpty()) { continue; }
            if (line.trimmed().startsWith("Icon=")) {
                result = line.trimmed().replace("Icon=", "");
                break;
            }
        }
        file.close();
        return result;
    }
    static QString findIconInDir(QString dir, QString icon)
    {
        QString result;
        if (dir.isEmpty() || icon.isEmpty()) { return result; }

        QString theme = QIcon::themeName();
        if (theme.isEmpty()) { theme = "hicolor"; }

        QStringList iconSizes;
        iconSizes << "128" << "64" << "48" << "32" << "22" << "16";

        // theme
        QDirIterator it(QString("%1/%2").arg(dir).arg(theme), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString found = it.next();
            if (found.split("/").takeLast().split(".").takeFirst()==icon) {
                for (int i=0;i<iconSizes.size();++i) {
                    QString hasFile = found.replace(QRegExp("/[.*]x[.*]/"),QString("/%1x%1/").arg(iconSizes.at(i)));
                    if (QFile::exists(hasFile)) { return hasFile; }
                }
                return found;
            }
        }
        // hicolor
        if (theme!="hicolor") {
            QDirIterator hicolor(QString("%1/%2").arg(dir).arg("hicolor"), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (hicolor.hasNext()) {
                QString found = hicolor.next();
                if (found.split("/").takeLast().split(".").takeFirst()==icon) {
                    for (int i=0;i<iconSizes.size();++i) {
                        QString hasFile = found.replace(QRegExp("/[.*]x[.*]/"),QString("/%1x%1/").arg(iconSizes.at(i)));
                        if (QFile::exists(hasFile)) { return hasFile; }
                    }
                    return found;
                }
            }
        }
        // pixmaps
        for (int i=0;i<pixmapLocations().size();++i) {
            QDirIterator pixmaps(pixmapLocations().at(i), QStringList() << "*.png" << "*.jpg" << "*.xpm", QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (pixmaps.hasNext()) {
                QString found = pixmaps.next();
                if (found.split("/").takeLast().split(".").takeFirst()==icon) { return found; }
            }
        }
        return result;
    }
    static QString findIcon(QString fileIcon)
    {
        QString result;
        if (fileIcon.isEmpty()) { return result; }
        for (int i=0;i<iconLocations().size();++i) {
            QString icon = findIconInDir(iconLocations().at(i), fileIcon);
            if (!icon.isEmpty()) { return icon; }
        }
        return result;
    }
    static QString findApplication(QString desktopFile)
    {
        QString result;
        if (desktopFile.isEmpty()) { return result; }
        for (int i=0;i<applicationLocations().size();++i) {
            QDirIterator it(applicationLocations().at(i), QStringList("*.desktop"), QDir::Files|QDir::NoDotAndDotDot);
            while (it.hasNext()) {
                QString found = it.next();
                if (found.split("/").takeLast()==desktopFile) {
                    qDebug() << "found app" << found;
                    return found;
                }
            }
        }
        return result;
    }
    static QString findApplicationIcon(QString app)
    {
        QString result;
        QString desktop = findApplication(app);
        if (desktop.isEmpty()) { return result; }
        QString icon = getDesktopIcon(desktop);
        if (icon.isEmpty()) { return result; }
        result = findIcon(icon);
        return result;
    }
};

#endif // COMMON_H
