#include "settingsdialog.h"
#include "icondlg.h"
#include "fileutils.h"
#include "applicationdialog.h"
#include "properties.h"
#include "common.h"

#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QFormLayout>

SettingsDialog::SettingsDialog(QSettings *settings,
                               MimeUtils *mimeUtils,
                               QWidget *parent) : QDialog(parent)
{

    // setup icon theme search path
    QStringList iconsPath = QIcon::themeSearchPaths();
    QString iconsHomeLocal = QString("%1/.local/share/icons").arg(QDir::homePath());
    QString iconsHome = QString("%1/.icons").arg(QDir::homePath());
    if (QFile::exists(iconsHomeLocal) && !iconsPath.contains(iconsHomeLocal)) { iconsPath.prepend(iconsHomeLocal); }
    if (QFile::exists(iconsHome) && !iconsPath.contains(iconsHome)) { iconsPath.prepend(iconsHome); }
    iconsPath << QString("%1/../share/icons").arg(qApp->applicationDirPath());
    QIcon::setThemeSearchPaths(iconsPath);
    qDebug() << "using icon theme search path" << QIcon::themeSearchPaths();
    Common::setupIconTheme(qApp->applicationFilePath());

    this->settingsPtr = settings;
    this->mimeUtilsPtr = mimeUtils;

    setWindowIcon(QIcon::fromTheme("applications-other"));
    setWindowTitle(tr("Default Applications"));

    QDialogButtonBox* btns = new QDialogButtonBox(this);
    btns->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(btns, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btns, SIGNAL(rejected()), this, SLOT(reject()));

    this->setMinimumWidth(640);
    this->setMinimumHeight(480);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->addWidget(createMimeSettings());
    layoutMain->addWidget(btns);

    // Read settings
    QTimer::singleShot(100, this, SLOT(readSettings()));
}

QWidget* SettingsDialog::createMimeSettings()
{

  // Widget and its layout
  QWidget *widget = new QWidget();
  QVBoxLayout* layoutWidget = new QVBoxLayout(widget);

  // Shortcuts group box
  QGroupBox* grpMimes = new QGroupBox(tr("Mime types"), widget);
  QVBoxLayout *layoutMimes = new QVBoxLayout(grpMimes);

  // Editation of application list
  grpAssoc = new QGroupBox(tr("Applications"), widget);
  grpAssoc->setEnabled(false);
  QGridLayout* layoutAssoc = new QGridLayout(grpAssoc);
  listAssoc = new QListWidget(grpAssoc);
  QPushButton* btnAdd = new QPushButton(/*tr("Add.."), */grpAssoc);
  QPushButton* btnRem = new QPushButton(/*tr("Remove"),*/ grpAssoc);
  QPushButton* btnUp = new QPushButton(/*tr("Move up"),*/ grpAssoc);
  QPushButton* btnDown = new QPushButton(/*tr("Move down"), */grpAssoc);
  layoutAssoc->addWidget(listAssoc, 0, 0, 4, 1);
  layoutAssoc->addWidget(btnAdd, 0, 1);
  layoutAssoc->addWidget(btnRem, 1, 1);
  layoutAssoc->addWidget(btnUp, 2, 1);
  layoutAssoc->addWidget(btnDown, 3, 1);

  btnAdd->setIcon(QIcon::fromTheme("list-add"));
  btnRem->setIcon(QIcon::fromTheme("edit-delete"));
  btnUp->setIcon(QIcon::fromTheme("go-up"));
  btnDown->setIcon(QIcon::fromTheme("go-down"));

  // tree filter
  QLineEdit *mimeSearch = new QLineEdit(grpMimes);
  mimeSearch->setPlaceholderText(tr("Filter ..."));
  mimeSearch->setClearButtonEnabled(true);
  connect(mimeSearch, SIGNAL(textChanged(QString)), this, SLOT(filterMimes(QString)));
  layoutMimes->addWidget(mimeSearch);

  // Tree widget with list of shortcuts
  mimesWidget = new QTreeWidget(grpMimes);
  mimesWidget->setAlternatingRowColors(true);
  mimesWidget->setRootIsDecorated(true);
  mimesWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  QTreeWidgetItem *header = mimesWidget->headerItem();
  header->setText(0, tr("Mime"));
  header->setText(1, tr("Application"));
  mimesWidget->setColumnWidth(0, 220);
  layoutMimes->addWidget(mimesWidget);

  // Default mime apps
  QGroupBox* grpDMime = new QGroupBox(tr("Default mime applications"), widget);
  QFormLayout* layoutDMime = new QFormLayout(grpDMime);
  cmbDefaultMimeApps = new QComboBox(grpDMime);
  cmbDefaultMimeApps->addItem("/.local/share/applications/mimeapps.list");
  cmbDefaultMimeApps->addItem("/.local/share/applications/defaults.list");
  cmbDefaultMimeApps->addItem("/.config/mimeapps.list");
  layoutDMime->addRow(tr("Configuration file: "), cmbDefaultMimeApps);

  layoutWidget->addWidget(grpMimes);
  layoutWidget->addWidget(grpAssoc);
  layoutWidget->addWidget(grpDMime);

  // Connect
  connect(mimesWidget,
          SIGNAL(currentItemChanged(QTreeWidgetItem*,
                                    QTreeWidgetItem*)),
          SLOT(onMimeSelected(QTreeWidgetItem*,
                              QTreeWidgetItem*)));
  connect(btnAdd,
          SIGNAL(clicked()),
          SLOT(showAppDialog()));
  connect(btnRem,
          SIGNAL(clicked()),
          SLOT(removeAppAssoc()));
  connect(btnUp,
          SIGNAL(clicked()),
          SLOT(moveAppAssocUp()));
  connect(btnDown,
          SIGNAL(clicked()),
          SLOT(moveAppAssocDown()));

  return widget;
}

void SettingsDialog::onMimeSelected(QTreeWidgetItem *current,
                                    QTreeWidgetItem *previous)
{
  // Store previously used associations
  updateMimeAssoc(previous);

  // Clear previously used associations
  listAssoc->clear();

  // Check if current is editable
  if (current->childCount() > 0) {
    grpAssoc->setEnabled(false);
    return;
  }

  // Enable editation
  grpAssoc->setEnabled(true);

  // Prepare source of icons
  QIcon defaultIcon = QIcon::fromTheme("application-x-executable");

  QStringList apps = mimesWidget->currentItem()->text(1).remove(" ").split(";");
  foreach (QString app, apps) {

    // Skip empty string
    if (app.compare("") == 0) {
      continue;
    }

    // Finds icon
    QIcon temp = QIcon::fromTheme(app).pixmap(16, 16);
    if (temp.isNull()) {
        QString foundIcon = Common::findApplicationIcon(qApp->applicationFilePath(), QIcon::themeName(), app + ".desktop");
        if (!foundIcon.isEmpty()) {
            temp = QIcon(foundIcon);
        } else {
            temp = defaultIcon;
        }
    }

    // Add application
    listAssoc->addItem(new QListWidgetItem(temp, app, listAssoc));
  }
}

void SettingsDialog::updateMimeAssoc(QTreeWidgetItem* item)
{
  if (item && item->childCount() == 0) {
    QStringList associations;
    for (int i = 0; i < listAssoc->count(); i++) {
      associations.append(listAssoc->item(i)->text());
    }
    item->setText(1, associations.join(";"));
  }
}

void SettingsDialog::showAppDialog()
{

  // Choose application
  ApplicationDialog *dialog = new ApplicationDialog(false, this);
  if (dialog->exec()) {

    // If application name is empty, exit
    if (dialog->getCurrentLauncher().isEmpty()) {
      return;
    }

    // Retrieve launcher name
    QString name = dialog->getCurrentLauncher();

    // If application with same name is already used, exit
    for (int i = 0; i < listAssoc->count(); i++) {
      if (listAssoc->item(i)->text().compare(name) == 0) {
        return;
      }
    }

    // Add new launcher to the list of launchers
    if (dialog->getCurrentLauncher().compare("") != 0) {
      QIcon icon = QIcon::fromTheme(name).pixmap(16, 16);
      listAssoc->addItem(new QListWidgetItem(icon, name, listAssoc));
      updateMimeAssoc(mimesWidget->currentItem());
    }
  }
}

void SettingsDialog::removeAppAssoc()
{
  qDeleteAll(listAssoc->selectedItems());
  updateMimeAssoc(mimesWidget->currentItem());
}

void SettingsDialog::moveAppAssocUp()
{
  QListWidgetItem *current = listAssoc->currentItem();
  int currIndex = listAssoc->row(current);
  QListWidgetItem *prev = listAssoc->item(listAssoc->row(current) - 1);
  int prevIndex = listAssoc->row(prev);
  QListWidgetItem *temp = listAssoc->takeItem(prevIndex);
  listAssoc->insertItem(prevIndex, current);
  listAssoc->insertItem(currIndex, temp);
  updateMimeAssoc(mimesWidget->currentItem());
}

void SettingsDialog::moveAppAssocDown()
{
  QListWidgetItem *current = listAssoc->currentItem();
  int currIndex = listAssoc->row(current);
  QListWidgetItem *next = listAssoc->item(listAssoc->row(current) + 1);
  int nextIndex = listAssoc->row(next);
  QListWidgetItem *temp = listAssoc->takeItem(nextIndex);
  listAssoc->insertItem(currIndex, temp);
  listAssoc->insertItem(nextIndex, current);
  updateMimeAssoc(mimesWidget->currentItem());
}

void SettingsDialog::filterMimes(QString filter)
{
    qDebug() << "filter mimes" << filter;
    mimesWidget->setUpdatesEnabled(false);
    for (int i=0;i<mimesWidget->topLevelItemCount();++i) {
        QTreeWidgetItem *topItem = mimesWidget->topLevelItem(i);
        if (!topItem) { continue; }
        for (int y=0;y<topItem->childCount();++y) {
            QTreeWidgetItem *item = topItem->child(y);
            if (!item) { continue; }
            if (item->text(0).contains(filter) || filter.isEmpty()) {
                item->setHidden(false);
            } else { item->setHidden(true); }
        }
    }
    if (filter.isEmpty()) { mimesWidget->collapseAll(); }
    else { mimesWidget->expandAll(); }
    mimesWidget->setUpdatesEnabled(true);
    mimesWidget->update();
}

void SettingsDialog::readSettings()
{
  QString tmp = settingsPtr->value("defMimeAppsFile", MIME_APPS).toString();
#if QT_VERSION >= 0x050000
  cmbDefaultMimeApps->setCurrentText(tmp);
#else
  cmbDefaultMimeApps->setEditText(tmp);
#endif
  mimeUtilsPtr->setDefaultsFileName(cmbDefaultMimeApps->currentText());

  loadMimes();

}

void SettingsDialog::loadMimes()
{

  // Load list of mimes
  QStringList mimes = mimeUtilsPtr->getMimeTypes();

  // Default icon
  QIcon defaultIcon = QIcon::fromTheme("text-x-generic");

  // Mime categories and their icons
  QMap<QString, QTreeWidgetItem*> categories;
  QMap<QTreeWidgetItem*, QIcon> genericIcons;

  // Load mime settings
  foreach (QString mime, mimes) {

    QApplication::processEvents();

    // Skip all 'inode' nodes including 'inode/directory'
    if (mime.startsWith("inode")) {
      continue;
    }

    // Skip all 'x-content' and 'message' nodes
    if (/*mime.startsWith("x-content") ||*/ mime.startsWith("message")) {
      continue;
    }

    // Parse mime
    QStringList splitMime = mime.split("/");

    // Retrieve categories
    QIcon icon = defaultIcon;
    QString categoryName = splitMime.first();
    QTreeWidgetItem* category = categories.value(categoryName, Q_NULLPTR);
    if (!category) {
      category = new QTreeWidgetItem(mimesWidget);
      category->setText(0, categoryName);
      category->setFlags(Qt::ItemIsEnabled);
      categories.insert(categoryName, category);
      genericIcons.insert(category, icon);
    }

    // Load icon and default application for current mime
    // NOTE: if icon is not found generic icon is used
    QString appNames = mimeUtilsPtr->getDefault(mime).join(";");

    // Create item from current mime
    QTreeWidgetItem *item = new QTreeWidgetItem(category);
    item->setIcon(0, icon);
    item->setText(0, splitMime.at(1));
    item->setText(1, appNames.remove(".desktop"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
}

bool SettingsDialog::saveSettings()
{
  for (int i = 0; i < mimesWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem* category = mimesWidget->topLevelItem(i);
    QString categoryName = category->text(0) + "/";
    for (int j = 0; j < category->childCount(); j++) {
      QString mime = categoryName + category->child(j)->text(0);
      QString appNames = category->child(j)->text(1);
      if (!appNames.isEmpty()) {
        QStringList temps = appNames.split(";");
        for (int i = 0; i < temps.size(); i++) {
          temps[i] = temps[i] + ".desktop";
        }
        mimeUtilsPtr->setDefault(mime, temps);
      }
    }
  }
  mimeUtilsPtr->saveDefaults();

  // Save succeded
  return true;
}

void SettingsDialog::accept()
{
  if (saveSettings()) this->done(1);
}

void SettingsDialog::getIcon(QTreeWidgetItem* item, int column)
{
  if (column == 2) {
    icondlg *icons = new icondlg;
    if (icons->exec() == 1) {
      item->setText(column, icons->result);
      item->setIcon(column, QIcon::fromTheme(icons->result));
    }
    delete icons;
  }
  return;
}
