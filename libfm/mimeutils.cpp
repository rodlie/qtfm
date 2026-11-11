#include "mimeutils.h"
#include "fileutils.h"

#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDir>
#include <QApplication>

#ifdef Q_OS_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

#include "common.h"


/**
 * @brief Creates mime utils
 * @param parent
 */
MimeUtils::MimeUtils(QObject *parent) : QObject(parent) {
  defaultsFileName = MIME_APPS;
  defaults = new Properties();
  loadDefaults();
}
//---------------------------------------------------------------------------

/**
 * @brief Loads list of default applications for mimes
 * @return properties with default applications
 */
void MimeUtils::loadDefaults() {
  defaults->load(QDir::homePath() + defaultsFileName, "Default Applications");
  defaultsChanged = false;
}
//---------------------------------------------------------------------------

/**
 * @brief Destructor
 */
MimeUtils::~MimeUtils() {
  delete defaults;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns mime type of given file
 * @note This operation is slow, prevent its mass application
 * @param path path to file
 * @return mime type
 */
QString MimeUtils::getMimeType(const QString &path) {
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(path);
    //qDebug() << "mime type" << type.name() << path;
    return type.name();
}
//---------------------------------------------------------------------------

/**
 * @brief Returns list of mime types
 * @return list of available mimetypes
 */
QStringList MimeUtils::getMimeTypes() const {
    QStringList result = Common::getMimeTypes(qApp->applicationFilePath());
    //qDebug() << "getMimeTypes"  << result;
    return result;
}
//---------------------------------------------------------------------------

/**
 * @brief Opens file in a default application
 * @param file
 * @param processOwner
 */
void MimeUtils::openInApp(const QFileInfo &file, QString termCmd) {
    qDebug() << "openInApp without app";
  QString mime = getMimeType(file.absoluteFilePath());
  QString app = defaults->value(mime).toString().split(";").first();
  if (app.isEmpty() && mime.startsWith("text/") && mime != "text/plain") {
      // fallback for text
      app = defaults->value("text/plain").toString().split(";").first();
  }
  QString desktop = Common::findApplication(qApp->applicationFilePath(), app);
  qDebug() << "openInApp" << file.absoluteFilePath() << termCmd << mime << app << desktop;
  if (!desktop.isEmpty()) {
    DesktopFile df = DesktopFile(desktop);
    if (!df.isTerminal()) { termCmd.clear(); }
    else {
        if (termCmd.isEmpty()) { termCmd = "xterm"; }
    }
    openInApp(df.getExec(), file, termCmd);
  } else {
#ifdef Q_OS_DARWIN
      CFURLRef ref = CFURLCreateWithFileSystemPath(nullptr,
                                                   file.absoluteFilePath().toCFString(),
                                                   kCFURLPOSIXPathStyle,
                                                   file.isDir());
      LSOpenCFURLRef(ref, nullptr);
#else

     QString title = tr("No default application");
     QString msg = tr("No default application for mime: %1!").arg(mime);
     QMessageBox::warning(nullptr, title, msg);
#endif
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Opens file in a given application
 * @param exe name of application to be executed
 * @param file to be opened in executed application
 * @param processOwner process owner (default NULL)
 */
void MimeUtils::openInApp(QString exe, const QFileInfo &file,
                          QString termCmd) {

  qDebug() << "openInApp" << exe << file.absoluteFilePath() << termCmd;

  // This is not the right the solution, but qpdfview won't start otherwise
  // TODO: Repair it correctly
  if (exe.contains("qpdfview")) {
    exe = "qpdfview";
  }

  startApp(exe, QStringList() << file.filePath(), termCmd);
}

void MimeUtils::openFilesInApp(QString exe, const QStringList &files, QString termCmd)
{
  startApp(exe, files, termCmd);
}

/**
 * @brief Starts an application (desktop entry Exec line) with given files
 *
 * The Exec line is split into a program and its arguments, the field codes
 * (%f %F %u %U) are replaced by the files and %i %c %k are dropped. If the
 * Exec line has no file field code, the files are appended. Every file is
 * passed as its own argument, so no quoting is needed.
 *
 * @param exe Exec line of the application
 * @param files files to be opened
 * @param termCmd terminal command (may contain arguments); empty to run
 * the application directly
 */
void MimeUtils::startApp(const QString &exe, const QStringList &files,
                         const QString &termCmd) {
  QStringList appArgs = QProcess::splitCommand(exe);
  if (appArgs.isEmpty()) { return; }
  const QString name = appArgs.takeFirst();
  const QString first = files.isEmpty() ? QString() : files.first();

  QStringList args;
  bool hasFileCode = false;
  for (const QString &token : appArgs) {
    if (token == "%f" || token == "%u") {
      if (!first.isEmpty()) { args << first; }
      hasFileCode = true;
    } else if (token == "%F" || token == "%U") {
      args << files;
      hasFileCode = true;
    } else if (token == "%i" || token == "%c" || token == "%k") {
      continue;
    } else {
      QString arg = token;
      if (arg.contains("%f") || arg.contains("%u")) {
        arg.replace("%f", first).replace("%u", first);
        hasFileCode = true;
      }
      if (arg.contains("%F") || arg.contains("%U")) {
        arg.replace("%F", files.join(" ")).replace("%U", files.join(" "));
        hasFileCode = true;
      }
      arg.replace("%%", "%");
      args << arg;
    }
  }
  if (!hasFileCode) { args << files; }

  QString program = name;
  if (!termCmd.isEmpty()) {
    QStringList term = QProcess::splitCommand(termCmd);
    if (term.isEmpty()) { term << "xterm"; }
    program = term.takeFirst();
    args = term << "-e" << name << args;
  }

  qDebug() << "qprocess start detached" << program << args;
  QProcess::startDetached(program, args);
}
//---------------------------------------------------------------------------

/**
 * @brief Sets defaults file name (name of file where defaults are stored)
 * @param fileName
 */
void MimeUtils::setDefaultsFileName(const QString &fileName) {
  this->defaultsFileName = fileName;
  loadDefaults();
}
//---------------------------------------------------------------------------

/**
 * @brief Returns defaults file name
 * @return name of file where defaults are stored
 */
QString MimeUtils::getDefaultsFileName() const {
    return defaultsFileName;
}

QString MimeUtils::getAppForMimeType(const QString &mime) const
{
    return defaults->value(mime).toString().split(";").first();
}
//---------------------------------------------------------------------------

/**
 * @brief Generates default application-mime associations
 */
void MimeUtils::generateDefaults() {

  // Load list of applications
  QList<DesktopFile> apps = FileUtils::getApplications();
  QStringList names;

  // Find defaults; for each application...
  // ------------------------------------------------------------------------
  foreach (DesktopFile a, apps) {

  // ignore NoDisplay=true
  if (a.noDisplay()) { continue; }

    // For each mime of current application...
    QStringList mimes = a.getMimeType();
    foreach (QString mime, mimes) {
      if (mime.trimmed().isEmpty()) { continue; }

      // Current app name
      QString name = a.getPureFileName() + ".desktop";
      names.append(name);

      // If current mime is not mentioned in the list of defaults, add it
      // together with current application and continue
      if (!defaults->contains(mime)) {
        defaults->set(mime, name);
        defaultsChanged = true;
        continue;
      }

      // Retrieve list of default applications for current mime, if it does
      // not contain current application, add this application to list
      QStringList appNames = defaults->value(mime).toString().split(";");
      if (!appNames.contains(name)) {
        appNames.append(name);
        defaults->set(mime, appNames.join(";"));
        defaultsChanged = true;
      }
    }
  }

  // Delete dead defaults (non existing apps)
  // ------------------------------------------------------------------------
  foreach (QString key, defaults->getKeys()) {
    QStringList tmpNames1 = defaults->value(key).toString().split(";");
    QStringList tmpNames2 = QStringList();
    foreach (QString name, tmpNames1) {
      if (names.contains(name)) {
        tmpNames2.append(name);
      }
    }
    if (key.trimmed().isEmpty() || tmpNames2.isEmpty()) {
      // no mime or no application left: an entry like "mime=" is useless
      defaults->remove(key);
      defaultsChanged = true;
    } else if (tmpNames1.size() != tmpNames2.size()) {
      defaults->set(key, tmpNames2.join(";"));
      defaultsChanged = true;
    }
  }

  // Save defaults if changed
  saveDefaults();
}
//---------------------------------------------------------------------------

/**
 * @brief Sets default mime association
 * @param mime mime name
 * @param apps list of applications (desktop file names)
 */
void MimeUtils::setDefault(const QString &mime, const QStringList &apps) {
  QString value = apps.join(";");
  if (value.compare(defaults->value(mime, "").toString()) != 0) {
    // without applications there is nothing to store
    if (value.isEmpty() || mime.trimmed().isEmpty()) { defaults->remove(mime); }
    else { defaults->set(mime, value); }
    defaultsChanged = true;
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Returns default applications for given mime
 * @param mime
 * @return list of default applications name
 */
QStringList MimeUtils::getDefault(const QString &mime) const {
  return defaults->value(mime).toString().split(";");
}
//---------------------------------------------------------------------------

/**
 * @brief Saves defaults
 */
void MimeUtils::saveDefaults() {
  if (defaultsChanged) {
    defaults->save(QDir::homePath() + defaultsFileName, "Default Applications");
    defaultsChanged = false;
  }
}
//---------------------------------------------------------------------------
