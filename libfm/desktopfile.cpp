#include "desktopfile.h"
#include "properties.h"
#include <QFile>
#include <QDebug>

/**
 * @brief Loads desktop file
 * @param fileName
 */
DesktopFile::DesktopFile(const QString &fileName) {

  // Store file name
  this->fileName = fileName;

  // File validity
  if (!QFile::exists(fileName)) {
    return;
  }

  // Loads .desktop file (read from 'Desktop Entry' group)
  Properties desktop(fileName, "Desktop Entry");
  name = desktop.value("Name", "").toString();
  genericName = desktop.value("GenericName", "").toString();
  exec = desktop.value("Exec", "").toString();
  icon = desktop.value("Icon", "").toString();
  type = desktop.value("Type", "Application").toString();
  no_display = desktop.value("NoDisplay", false).toBool();
  terminal = desktop.value("Terminal", false).toBool();
  categories = desktop.value("Categories").toString().remove(" ").split(";");
  mimeType = desktop.value("MimeType").toString().remove(" ").split(";");

  // Fix categories
  if (categories.first().compare("") == 0) {
    categories.removeFirst();
  }
}
//---------------------------------------------------------------------------

QString DesktopFile::getFileName() const {
  return fileName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getPureFileName() const {
  return fileName.split("/").last().remove(".desktop");
}
//---------------------------------------------------------------------------

QString DesktopFile::getName() const {
    return name;
}

QString DesktopFile::getGenericName() const {
    return genericName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getExec() const {
  return exec;
}
//---------------------------------------------------------------------------

QString DesktopFile::getIcon() const {
  return icon;
}
//---------------------------------------------------------------------------

QString DesktopFile::getType() const {
    return type;
}

bool DesktopFile::noDisplay() const
{
    return no_display;
}

bool DesktopFile::isTerminal() const
{
     return terminal;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getCategories() const {
  return categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const {
  return mimeType;
}
//---------------------------------------------------------------------------
