#include "properties.h"

#include <QTextStream>
#include <QStringList>
#include <QFile>

/**
 * @brief Creates properties
 * @param fileName
 * @param group
 */
Properties::Properties(const QString &fileName, const QString &group) {
  if (!fileName.isEmpty()) {
    load(fileName, group);
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Creates properties
 * @param other properties
 */
Properties::Properties(const Properties &other) {
  this->data = other.data;
}
//---------------------------------------------------------------------------


/**
 * @brief Loads property file
 * @param fileName
 * @param group
 * @return true if load was successful
 */
bool Properties::load(const QString &fileName, const QString &group) {

  // NOTE: This class is used for reading of property files instead of QSettings
  // class, which considers separator ';' as comment

  // Try open file
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  // Clear old data
  data.clear();

  // Indicator whether group was found or not, if name of group was not
  // specified, groupFound is always true
  bool groupFound = group.isEmpty();

  // Read properties
  QTextStream in(&file);
  while (!in.atEnd()) {

    // Read new line
    QString line = in.readLine();

    // Skip empty line or line with invalid format
    if (line.trimmed().isEmpty()) {
      continue;
    }

    // Read group
    // NOTE: symbols '[' and ']' can be found not only in group names, but
    // only group can start with '['
    if (!group.isEmpty() && line.trimmed().startsWith("[")) {
      QString tmp = line.trimmed().replace("[", "").replace("]", "");
      groupFound = group.trimmed().compare(tmp) == 0;
    }

    // If we are in correct group and line contains assignment then read data
    if (groupFound && line.contains("=")) {
        int index = line.indexOf("=");
        QString key = line.mid(0, index);
        QString val = line.mid(index+1);
        data.insert(key, val);
    }
  }
  file.close();
  return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Saves properties to file
 * @param fileName
 * @param group
 * @return true if success
 */
bool Properties::save(const QString &fileName, const QString &group) {

  // Only the given group is rewritten: keep the lines the file holds before
  // and after it (other groups, e.g. "Added Associations" in mimeapps.list)
  QStringList before, after;
  if (!group.isEmpty()) {
    QFile old(fileName);
    if (old.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&old);
      bool inGroup = false;
      bool groupSeen = false;
      while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().startsWith("[")) {
          QString tmp = line.trimmed().replace("[", "").replace("]", "");
          inGroup = group.trimmed().compare(tmp) == 0;
          if (inGroup) { groupSeen = true; continue; }
        }
        if (inGroup) { continue; }
        if (groupSeen) { after.append(line); } else { before.append(line); }
      }
      old.close();
    }
    while (!before.isEmpty() && before.last().trimmed().isEmpty()) { before.removeLast(); }
    while (!after.isEmpty() && after.last().trimmed().isEmpty()) { after.removeLast(); }
  }

  // Try open file
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream out(&file);
  foreach (QString line, before) { out << line << "\n"; }
  if (!before.isEmpty()) { out << "\n"; }

  // Write group
  if (!group.isEmpty()) {
    out << "[" + group + "]\n";
  }

  // Write data
  foreach (QString key, data.keys()) {
    if (key.trimmed().isEmpty()) { continue; } // not a valid entry
    out << key << "=" << data.value(key).toString() << "\n";
  }

  if (!after.isEmpty()) { out << "\n"; }
  foreach (QString line, after) { out << line << "\n"; }

  // Exit
  file.close();
  return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns true if property with given key is present in properties
 * @param key
 * @return true if property with given key is present in properties
 */
bool Properties::contains(const QString &key) const {
  return data.contains(key);
}
//---------------------------------------------------------------------------

/**
 * @brief Returns value
 * @param key
 * @param defaultValue
 * @return value
 */
QVariant Properties::value(const QString &key, const QVariant &defaultValue) {
  return data.value(key, defaultValue);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets value to properties
 * @param key
 * @param value
 */
void Properties::remove(const QString &key) {
  data.remove(key);
}
//---------------------------------------------------------------------------

void Properties::set(const QString &key, const QVariant &value) {
  if (data.contains(key)) {
    data.take(key);
  }
  data.insert(key, value);
}
//---------------------------------------------------------------------------

/**
 * @brief Returns keys (names of properties)
 * @param key
 * @param value
 */
QStringList Properties::getKeys() const {
  return data.keys();
}
//---------------------------------------------------------------------------
