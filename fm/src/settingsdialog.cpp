#include "settingsdialog.h"
#include "icondlg.h"
#include "fileutils.h"
#include "applicationdialog.h"
#include "properties.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QStringList>

/**
 * @brief Creates settings dialog
 * @param actionList
 * @param settings
 * @param mimeUtils
 * @param parent
 */
SettingsDialog::SettingsDialog(QList<QAction *> *actionList,
                               QSettings *settings, MimeUtils *mimeUtils,
                               QWidget *parent) : QDialog(parent) {

  // Store pointer to custom action manager
  this->actionListPtr = actionList;
  this->settingsPtr = settings;
  this->mimeUtilsPtr = mimeUtils;

  // Main widgets of this dialog
  selector = new QListWidget(this);
  stack = new QStackedWidget(this);
  selector->setMaximumWidth(150);
  stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Buttons
  QDialogButtonBox* btns = new QDialogButtonBox(this);
  btns->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
  connect(btns, SIGNAL(accepted()), this, SLOT(accept()));
  connect(btns, SIGNAL(rejected()), this, SLOT(reject()));

  // Size
  this->setMinimumWidth(640);
  this->setMinimumHeight(480);

  // Layouts
  QHBoxLayout* layoutMain = new QHBoxLayout(this);
  QVBoxLayout* layoutRight = new QVBoxLayout();
  layoutMain->addWidget(selector);
  layoutMain->addItem(layoutRight);
  layoutRight->addWidget(stack);
  layoutRight->addItem(new QSpacerItem(0, 10));
  layoutRight->addWidget(btns);

  // Icons
  QIcon icon1 = QIcon::fromTheme("system-file-manager");
  QIcon icon2 = QIcon::fromTheme("applications-system");
  QIcon icon3 = QIcon::fromTheme("accessories-character-map");
  QIcon icon4 = QIcon::fromTheme("preferences-desktop-filetype-association");

  // Add widget with configurations
  selector->setMinimumWidth(160);
  selector->setViewMode(QListView::ListMode);
  selector->setIconSize(QSize(32, 32));
  selector->addItem(new QListWidgetItem(icon1, tr("General"), selector));
  selector->addItem(new QListWidgetItem(icon2, tr("Custom actions"), selector));
  selector->addItem(new QListWidgetItem(icon3, tr("Shortcuts"), selector));
  selector->addItem(new QListWidgetItem(icon4, tr("Mime types"), selector));

  stack->addWidget(createGeneralSettings());
  stack->addWidget(createActionsSettings());
  stack->addWidget(createShortcutSettings());
  stack->addWidget(createMimeProgress());
  stack->addWidget(createMimeSettings());
  connect(selector, SIGNAL(currentRowChanged(int)), stack,
          SLOT(setCurrentIndex(int)));
  connect(selector, SIGNAL(currentRowChanged(int)), SLOT(loadMimes(int)));

  // Align items
  for (int i = 0; i < selector->count(); i++) {
    selector->item(i)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  }

  // Read settings
  readSettings();
}
//---------------------------------------------------------------------------

/**
 * @brief Creates widget with general settings
 * @return widget
 */
QWidget *SettingsDialog::createGeneralSettings() {

  // Main widget and layout
  QWidget* widget = new QWidget();
  QVBoxLayout* layoutWidget = new QVBoxLayout(widget);

  // Appearance
  QGroupBox* grpAppear = new QGroupBox(tr("Appearance"), widget);
  QFormLayout* layoutAppear = new QFormLayout(grpAppear);
  checkThumbs = new QCheckBox(grpAppear);
  checkHidden = new QCheckBox(grpAppear);
  checkTabs = new QCheckBox(grpAppear);
  //cmbIconTheme = new QComboBox(grpAppear);
  //layoutAppear->addRow(tr("Icon theme:"), cmbIconTheme);
  layoutAppear->addRow(tr("Show thumbnails: "), checkThumbs);
  layoutAppear->addRow(tr("Show hidden files: "), checkHidden);
  layoutAppear->addRow(tr("Tabs on top: "), checkTabs);

  // Confirmation
  QGroupBox* grpConfirm = new QGroupBox(tr("Confirmation"), widget);
  QFormLayout* layoutConfirm = new QFormLayout(grpConfirm);
  checkDelete = new QCheckBox(grpConfirm);
  layoutConfirm->addRow(tr("Ask before file is deleted: "), checkDelete);

  // Terminal emulator
  /*QGroupBox* grpTerm = new QGroupBox(tr("Terminal emulator"), widget);
  QFormLayout* layoutTerm = new QFormLayout(grpTerm);
  editTerm = new QLineEdit(grpTerm);
  layoutTerm->addRow(tr("Command: "), editTerm);*/

  // Default mime apps
  QGroupBox* grpDMime = new QGroupBox(tr("Default mime applications"), widget);
  QFormLayout* layoutDMime = new QFormLayout(grpDMime);
  cmbDefaultMimeApps = new QComboBox(grpDMime);
  cmbDefaultMimeApps->addItem("/.local/share/applications/mimeapps.list");
  cmbDefaultMimeApps->addItem("/.local/share/applications/defaults.list");
  layoutDMime->addRow(tr("Configuration file: "), cmbDefaultMimeApps);

  // Layout of widget
  layoutWidget->addWidget(grpAppear);
  layoutWidget->addWidget(grpConfirm);
  //layoutWidget->addWidget(grpTerm);
  layoutWidget->addWidget(grpDMime);
  layoutWidget->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                              QSizePolicy::MinimumExpanding));
  return widget;
}
//---------------------------------------------------------------------------

/**
 * @brief Creates widget with custom actions settings
 * @return widget
 */
QWidget* SettingsDialog::createActionsSettings() {

  // Widget
  QWidget* widget = new QWidget(this);

  // Options group box
  QGroupBox* grpOptions = new QGroupBox(tr("Options"), widget);
  checkOutput = new QCheckBox(grpOptions);
  QFormLayout* layoutOptions = new QFormLayout(grpOptions);
  layoutOptions->addRow(tr("Show dialog with action's output:"), checkOutput);

  // Actions group box
  QGroupBox* grpMain = new QGroupBox(tr("Custom actions"), widget);
  QVBoxLayout* mainLayout = new QVBoxLayout(grpMain);

  // Create actions widget
  actionsWidget = new QTreeWidget(grpMain);
  actionsWidget->setAlternatingRowColors(true);
  actionsWidget->setRootIsDecorated(false);
  actionsWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  actionsWidget->setColumnWidth(1, 160);
  actionsWidget->setColumnWidth(2, 160);
  actionsWidget->setDragDropMode(QAbstractItemView::InternalMove);
  mainLayout->addWidget(actionsWidget);

  // Create header of actions widget
  QTreeWidgetItem *header = actionsWidget->headerItem();
  header->setText(0, tr("Filetype"));
  header->setText(1, tr("Text"));
  header->setText(2, tr("Icon"));
  header->setText(3, tr("Command"));

  // Connect action widget
  connect(actionsWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),
          this, SLOT(getIcon(QTreeWidgetItem *,int)));
  connect(actionsWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
          this, SLOT(onActionChanged(QTreeWidgetItem*,int)));

  // Create control buttons
  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  addButton = new QToolButton();
  delButton = new QToolButton();
  infoButton = new QToolButton();
  addButton->setIcon(QIcon::fromTheme("list-add"));
  delButton->setIcon(QIcon::fromTheme("list-remove"));
  infoButton->setIcon(QIcon::fromTheme("dialog-question",
                                       QIcon::fromTheme("help-browser")));

  // Connect buttons
  connect(addButton, SIGNAL(clicked()), this, SLOT(addCustomAction()));
  connect(delButton, SIGNAL(clicked()), this, SLOT(delCustomAction()));
  connect(infoButton, SIGNAL(clicked()), this, SLOT(infoCustomAction()));

  // Layouts
  horizontalLayout->addWidget(infoButton);
  horizontalLayout->addWidget(addButton);
  horizontalLayout->addWidget(delButton);
  horizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
  mainLayout->addLayout(horizontalLayout);

  // Outer layout
  QVBoxLayout *outerLayout = new QVBoxLayout(widget);
  outerLayout->addWidget(grpMain);
  outerLayout->addWidget(grpOptions);

  return widget;
}
//---------------------------------------------------------------------------

/**
 * @brief Creates widget with shortcuts settings
 * @return widget
 */
QWidget* SettingsDialog::createShortcutSettings() {

  // Widget and its layout
  QWidget *widget = new QWidget();
  QVBoxLayout* layoutWidget = new QVBoxLayout(widget);

  // Shortcuts group box
  QGroupBox* grpShortcuts = new QGroupBox(tr("Configure shortcuts"), widget);
  QVBoxLayout *layoutShortcuts = new QVBoxLayout(grpShortcuts);

  // Tree widget with list of shortcuts
  shortsWidget = new QTreeWidget(grpShortcuts);
  shortsWidget->setAlternatingRowColors(true);
  shortsWidget->setRootIsDecorated(false);
  shortsWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  QTreeWidgetItem *header = shortsWidget->headerItem();
  header->setText(0, tr("Action"));
  header->setText(1, tr("Shortcut"));
  shortsWidget->setColumnWidth(0, 220);
  layoutShortcuts->addWidget(shortsWidget);
  layoutWidget->addWidget(grpShortcuts);

  return widget;
}
//---------------------------------------------------------------------------

/**
 * @brief Creates widget with mime progress bar
 * @return widget
 */
QWidget* SettingsDialog::createMimeProgress() {

  // Widget and its layout
  QWidget* widget = new QWidget();
  QGridLayout* layout = new QGridLayout(widget);

  // Mime progress bar
  progressMime = new QProgressBar(widget);
  progressMime->setMinimumWidth(250);
  progressMime->setMaximumWidth(250);
  layout->addWidget(new QLabel(tr("Loading mime types..."), widget), 1, 1);
  layout->addWidget(progressMime, 2, 1);
  layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 0, 4);
  layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 2, 4);
  layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                  QSizePolicy::MinimumExpanding), 0, 1);
  layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                  QSizePolicy::MinimumExpanding), 3, 1);

  return widget;
}
//---------------------------------------------------------------------------

/**
 * @brief Creates widget with mime settings
 * @return widget
 */
QWidget* SettingsDialog::createMimeSettings() {

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
  QPushButton* btnAdd = new QPushButton(tr("Add.."), grpAssoc);
  QPushButton* btnRem = new QPushButton(tr("Remove"), grpAssoc);
  QPushButton* btnUp = new QPushButton(tr("Move up"), grpAssoc);
  QPushButton* btnDown = new QPushButton(tr("Move down"), grpAssoc);
  layoutAssoc->addWidget(listAssoc, 0, 0, 4, 1);
  layoutAssoc->addWidget(btnAdd, 0, 1);
  layoutAssoc->addWidget(btnRem, 1, 1);
  layoutAssoc->addWidget(btnUp, 2, 1);
  layoutAssoc->addWidget(btnDown, 3, 1);

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
  layoutWidget->addWidget(grpMimes);
  layoutWidget->addWidget(grpAssoc);

  // Load application list
  QStringList apps = FileUtils::getApplicationNames();
  apps.replaceInStrings(".desktop", "");
  apps.sort();

  // Prepare source of icons
  QDir appIcons1(QString("%1/config/share/pixmaps").arg(QDir::homePath()),"", 0, QDir::Files | QDir::NoDotAndDotDot);
  QDir appIcons2(QString("%1/../share/pixmaps").arg(qApp->applicationFilePath()),"", 0, QDir::Files | QDir::NoDotAndDotDot);
  QDir appIcons3("/usr/share/pixmaps","", 0, QDir::Files | QDir::NoDotAndDotDot);
  QDir appIcons4("/usr/local/share/pixmaps","", 0, QDir::Files | QDir::NoDotAndDotDot);
  QStringList iconFiles;
  for (int i=0;i<appIcons1.entryList().size();++i) {
      iconFiles << QString("%1/%2").arg(appIcons1.absolutePath()).arg(appIcons1.entryList().at(i));
  }
  for (int i=0;i<appIcons2.entryList().size();++i) {
      iconFiles << QString("%1/%2").arg(appIcons2.absolutePath()).arg(appIcons2.entryList().at(i));
  }
  for (int i=0;i<appIcons3.entryList().size();++i) {
      iconFiles << QString("%1/%2").arg(appIcons3.absolutePath()).arg(appIcons3.entryList().at(i));
  }
  for (int i=0;i<appIcons4.entryList().size();++i) {
      iconFiles << QString("%1/%2").arg(appIcons4.absolutePath()).arg(appIcons4.entryList().at(i));
  }
  qDebug() << "icons" << iconFiles;
  /*QStringList iconFiles = appIcons1.entryList();
  iconFiles << appIcons2.entryList() << appIcons3.entryList() << appIcons4.entryList();
  iconFiles.removeDuplicates();
  qDebug() << "icons" << iconFiles;*/
  QIcon defaultIcon = QIcon::fromTheme("application-x-executable");

  // Loads icon list
  QList<QIcon> icons;
  foreach (QString app, apps) {
    QPixmap temp = QIcon::fromTheme(app).pixmap(16, 16);
    if (!temp.isNull()) {
      icons.append(temp);
    } else {
      QStringList searchIcons = iconFiles.filter(app);
      if (searchIcons.count() > 0) {
        qDebug() << "found icon" << searchIcons.at(0);
        icons.append(QIcon(/*"/usr/share/pixmaps/" + */searchIcons.at(0)));
      } else {
        icons.append(defaultIcon);
      }
    }
  }

  // Connect
  connect(mimesWidget,
          SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
          SLOT(onMimeSelected(QTreeWidgetItem*,QTreeWidgetItem*)));
  connect(btnAdd, SIGNAL(clicked()), SLOT(showAppDialog()));
  connect(btnRem, SIGNAL(clicked()), SLOT(removeAppAssoc()));
  connect(btnUp, SIGNAL(clicked()), SLOT(moveAppAssocUp()));
  connect(btnDown, SIGNAL(clicked()), SLOT(moveAppAssocDown()));

  return widget;
}
//---------------------------------------------------------------------------

/**
 * @brief Updates default applications editor
 * @param current
 * @param previous
 */
void SettingsDialog::onMimeSelected(QTreeWidgetItem *current,
                                    QTreeWidgetItem *previous) {

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

  // Prepare source of icons FIXME!
  qDebug() << "onmimeselected, fixme!";
  QDir appIcons("/usr/share/pixmaps","", 0, QDir::Files | QDir::NoDotAndDotDot);
  QStringList iconFiles = appIcons.entryList();
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
      QStringList searchIcons = iconFiles.filter(app);
      if (searchIcons.count() > 0) {
        temp = QIcon("/usr/share/pixmaps/" + searchIcons.at(0));
      } else {
        temp = defaultIcon;
      }
    }

    // Add application
    listAssoc->addItem(new QListWidgetItem(temp, app, listAssoc));
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Updates mime association
 * @param item
 */
void SettingsDialog::updateMimeAssoc(QTreeWidgetItem* item) {
  if (item && item->childCount() == 0) {
    QStringList associations;
    for (int i = 0; i < listAssoc->count(); i++) {
      associations.append(listAssoc->item(i)->text());
    }
    item->setText(1, associations.join(";"));
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Displays application chooser dialog
 */
void SettingsDialog::showAppDialog() {

  // Choose application
  ApplicationDialog *dialog = new ApplicationDialog(this);
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
//---------------------------------------------------------------------------

/**
 * @brief Removes association of application and mime type
 */
void SettingsDialog::removeAppAssoc() {
  qDeleteAll(listAssoc->selectedItems());
  updateMimeAssoc(mimesWidget->currentItem());
}
//---------------------------------------------------------------------------

/**
 * @brief Moves association up in list
 */
void SettingsDialog::moveAppAssocUp() {
  QListWidgetItem *current = listAssoc->currentItem();
  int currIndex = listAssoc->row(current);
  QListWidgetItem *prev = listAssoc->item(listAssoc->row(current) - 1);
  int prevIndex = listAssoc->row(prev);
  QListWidgetItem *temp = listAssoc->takeItem(prevIndex);
  listAssoc->insertItem(prevIndex, current);
  listAssoc->insertItem(currIndex, temp);
  updateMimeAssoc(mimesWidget->currentItem());
}
//---------------------------------------------------------------------------

/**
 * @brief Moves association down in list
 */
void SettingsDialog::moveAppAssocDown() {
  QListWidgetItem *current = listAssoc->currentItem();
  int currIndex = listAssoc->row(current);
  QListWidgetItem *next = listAssoc->item(listAssoc->row(current) + 1);
  int nextIndex = listAssoc->row(next);
  QListWidgetItem *temp = listAssoc->takeItem(nextIndex);
  listAssoc->insertItem(currIndex, temp);
  listAssoc->insertItem(nextIndex, current);
  updateMimeAssoc(mimesWidget->currentItem());
}
//---------------------------------------------------------------------------

/**
 * @brief Reads settings
 */
void SettingsDialog::readSettings() {

  // Read general settings
  checkThumbs->setChecked(settingsPtr->value("showThumbs", true).toBool());
  checkTabs->setChecked(settingsPtr->value("tabsOnTop", false).toBool());
  checkHidden->setChecked(settingsPtr->value("hiddenMode", true).toBool());
  checkDelete->setChecked(settingsPtr->value("confirmDelete", true).toBool());
  //editTerm->setText(settingsPtr->value("term").toString());

  // Load default mime appis location
  QString tmp = "/.local/share/applications/mimeapps.list";
  tmp = settingsPtr->value("defMimeAppsFile", tmp).toString();
#if QT_VERSION >= 0x050000
  cmbDefaultMimeApps->setCurrentText(tmp);
#else
  cmbDefaultMimeApps->setEditText(tmp);
#endif
  mimeUtilsPtr->setDefaultsFileName(cmbDefaultMimeApps->currentText());

  // Load icon themes
  /*QString currentTheme = settingsPtr->value("forceTheme").toString();
  QDirIterator it("/usr/share/icons", QDir::Dirs | QDir::NoDotAndDotDot);
  QStringList iconThemes;
  while (it.hasNext()) {
    it.next();
    iconThemes.append(it.fileName());
  }
  cmbIconTheme->addItems(iconThemes);
  cmbIconTheme->setCurrentIndex(iconThemes.indexOf(currentTheme));*/

  // Read custom actions
  checkOutput->setChecked(settingsPtr->value("showActionOutput", true).toBool());
  settingsPtr->beginGroup("customActions");
  QStringList keys = settingsPtr->childKeys();
  for (int i = 0; i < keys.count(); ++i) {
    QStringList temp = settingsPtr->value(keys.at(i)).toStringList();
    bool setChecked = 0;
    QString cmd = temp.at(3);
    if (cmd.at(0) == '|') {
      cmd.remove(0, 1);
      setChecked = 1;
    }
    QStringList itemData;
    itemData << temp.at(0) << temp.at(1) << temp.at(2) << cmd;
    QTreeWidgetItem *item = new QTreeWidgetItem(actionsWidget, itemData,0);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled
                   | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
    item->setCheckState(3, setChecked ? Qt::Checked : Qt::Unchecked);
  }
  settingsPtr->endGroup();

  // Add default actions
  /*if (keys.count() == 0) {
    QStringList def1, def2, def3;
    def1 << "gz,bz2" << tr("Extract here") << "package-x-generic" << "tar xf %f";
    def2 << "folder" << tr("Term here") << "terminal" << "urxvt -cd %F";
    def3 << "*" << tr("Compress") << "filesave" << "tar czf %n.tar.gz %f";
    QTreeWidgetItem *item1 = new QTreeWidgetItem(actionsWidget, def1, 0);
    QTreeWidgetItem *item2 = new QTreeWidgetItem(actionsWidget, def2, 0);
    QTreeWidgetItem *item3 = new QTreeWidgetItem(actionsWidget, def3, 0);
    item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable
                    | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable
                    | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item3->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable
                    | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
  }*/

  // Loads icons for actions
  for (int x = 0; x < actionsWidget->topLevelItemCount(); x++) {
    QString name = actionsWidget->topLevelItem(x)->text(2);
    actionsWidget->topLevelItem(x)->setIcon(2, QIcon::fromTheme(name));
  }

  // Read shortcuts
  readShortcuts();
}
//---------------------------------------------------------------------------

/**
 * @brief Reads shortcuts
 */
void SettingsDialog::readShortcuts() {

  // Delete list of shortcuts
  shortsWidget->clear();

  // Icon configuration
  QPixmap pixmap(16, 16);
  pixmap.fill(Qt::transparent);
  QIcon blank(pixmap);

  // Read shorcuts
  QHash<QString, QString> shortcuts;
  settingsPtr->beginGroup("customShortcuts");
  QStringList keys = settingsPtr->childKeys();
  for (int i = 0; i < keys.count(); ++i) {
    QStringList temp(settingsPtr->value(keys.at(i)).toStringList());
    shortcuts.insert(temp.at(0), temp.at(1));
  }
  settingsPtr->endGroup();

  // Assign shortcuts to action and bookmarks
  for (int i = 0; i < actionListPtr->count(); ++i) {
    QAction* act = actionListPtr->at(i);
    QString text = shortcuts.value(act->text());
    text = text.isEmpty() ? text : QKeySequence::fromString(text).toString();
    QStringList list;
    list << act->text() << text;
    QTreeWidgetItem *item = new QTreeWidgetItem(shortsWidget, list);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    item->setIcon(0, act->icon());
    if (item->icon(0).isNull()) {
      item->setIcon(0, blank);
    }
  }

  // Assign shortcuts to custom actions
  for (int i = 0; i < actionsWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *srcItem = actionsWidget->topLevelItem(i);
    QString text = shortcuts.value(srcItem->text(1));
    text = text.isEmpty() ? text : QKeySequence::fromString(text).toString();
    QStringList list;
    list << srcItem->text(1) << text;
    QTreeWidgetItem *item = new QTreeWidgetItem(shortsWidget, list);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    item->setIcon(0, QIcon::fromTheme(srcItem->text(2)));
    if (item->icon(0).isNull()) {
      item->setIcon(0, blank);
    }
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Loads mime types
 * @param section
 */
void SettingsDialog::loadMimes(int section) {

  // Mime progress section
  const int MIME_PROGRESS_SECTION = 3;

  // If section is not mime type configuration section exit
  if (section != MIME_PROGRESS_SECTION) {
    return;
  }

  // If mimes have been already loaded move to another section (mime config)
  if (mimesWidget->topLevelItemCount() > 0) {
    stack->setCurrentIndex(MIME_PROGRESS_SECTION + 1);
    return;
  }

  // Load list of mimes
  QStringList mimes = mimeUtilsPtr->getMimeTypes();

  // Init process
  progressMime->setRange(1, mimes.size());

  // Default icon
  QIcon defaultIcon = QIcon::fromTheme("unknown");

  // Mime cathegories and their icons
  QMap<QString, QTreeWidgetItem*> categories;
  QMap<QTreeWidgetItem*, QIcon> genericIcons;

  // Load mime settings
  foreach (QString mime, mimes) {

    // Updates progress
    progressMime->setValue(progressMime->value() + 1);

    // Skip all 'inode' nodes including 'inode/directory'
    if (mime.startsWith("inode")) {
      continue;
    }

    // Skip all 'x-content' and 'message' nodes
    if (mime.startsWith("x-content") || mime.startsWith("message")) {
      continue;
    }

    // Parse mime
    QStringList splitMime = mime.split("/");

    // Retrieve cathegory
    QIcon icon;
    QString categoryName = splitMime.first();
    QTreeWidgetItem* category = categories.value(categoryName, NULL);
    if (!category) {
      category = new QTreeWidgetItem(mimesWidget);
      category->setText(0, categoryName);
      category->setFlags(Qt::ItemIsEnabled);
      categories.insert(categoryName, category);
      icon = FileUtils::searchGenericIcon(categoryName, defaultIcon);
      genericIcons.insert(category, icon);
    } else {
      icon = genericIcons.value(category);
    }

    // Load icon and default application for current mime
    // NOTE: if icon is not found generic icon is used
    icon = FileUtils::searchMimeIcon(mime, icon);
    QString appNames = mimeUtilsPtr->getDefault(mime).join(";");

    // Create item from current mime
    QTreeWidgetItem *item = new QTreeWidgetItem(category);
    item->setIcon(0, icon);
    item->setText(0, splitMime.at(1));
    item->setText(1, appNames.remove(".desktop"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }

  // Move to mimes
  stack->setCurrentIndex(MIME_PROGRESS_SECTION + 1);
}
//---------------------------------------------------------------------------

/**
 * @brief Saves settings
 * @return true if successfull
 */
bool SettingsDialog::saveSettings() {

    qDebug() << "save settings dialog";
  // General settings
  // ------------------------------------------------------------------------
  settingsPtr->setValue("showThumbs", checkThumbs->isChecked());
  settingsPtr->setValue("tabsOnTop", checkTabs->isChecked());
  settingsPtr->setValue("hiddenMode", checkHidden->isChecked());
  settingsPtr->setValue("confirmDelete", checkDelete->isChecked());
  //settingsPtr->setValue("term", editTerm->text());
  //settingsPtr->setValue("forceTheme", cmbIconTheme->currentText());
  settingsPtr->setValue("defMimeAppsFile", cmbDefaultMimeApps->currentText());

  // Custom actions
  // ------------------------------------------------------------------------
  settingsPtr->setValue("showActionOutput", checkOutput->isChecked());
  settingsPtr->remove("customActions");
  settingsPtr->beginGroup("customActions");
  for (int i = 0; i < actionsWidget->topLevelItemCount(); i++) {
    QTreeWidgetItem *item = actionsWidget->topLevelItem(i);
    QStringList temp;
    QString cmd = item->text(3);
    if (item->checkState(3) == Qt::Checked) {
      cmd.prepend("|");
    }
    temp << item->text(0) << item->text(1) << item->text(2) << cmd;
    settingsPtr->setValue(QString(i),temp);
  }
  settingsPtr->endGroup();
  settingsPtr->setValue("customHeader", actionsWidget->header()->saveState());

  // Shortcuts
  // ------------------------------------------------------------------------
  QStringList shortcuts, duplicates;
  settingsPtr->remove("customShortcuts");
  settingsPtr->beginGroup("customShortcuts");
  for (int i = 0; i < shortsWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem *item = shortsWidget->topLevelItem(i);
    if (!item->text(1).isEmpty()) {
      int existing = shortcuts.indexOf(item->text(1));
      if (existing != -1) {
        duplicates.append(QString("<b>%1</b> - %2").arg(shortcuts.at(existing))
                          .arg(item->text(0)));
      }
      shortcuts.append(item->text(1));
      QStringList temp;
      temp << item->text(0) << item->text(1);
      settingsPtr->setValue(QString(shortcuts.count()), temp);
    }
  }
  settingsPtr->endGroup();

  // Mime types
  // ------------------------------------------------------------------------
  for (int i = 0; i < mimesWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem* cathegory = mimesWidget->topLevelItem(i);
    QString cathegoryName = cathegory->text(0) + "/";
    for (int j = 0; j < cathegory->childCount(); j++) {
      QString mime = cathegoryName + cathegory->child(j)->text(0);
      QString appNames = cathegory->child(j)->text(1);
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

  // Check for shortcuts duplicity
  // ------------------------------------------------------------------------
  if (duplicates.count()) {
    QString title = tr("Warning");
    QString msg = tr("Duplicate shortcuts detected:<p>%1");
    QMessageBox::information(this, title, msg.arg(duplicates.join("<p>")));
  }

  // Save succeded
  return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Accepts settings configuration
 */
void SettingsDialog::accept() {
  if (saveSettings()) this->done(1);
}
//---------------------------------------------------------------------------

/**
 * @brief Selects icon of custom action
 * @param item
 * @param column
 */
void SettingsDialog::getIcon(QTreeWidgetItem* item, int column) {
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
//---------------------------------------------------------------------------

/**
 * @brief Updates shortcuts
 * @param item
 * @param column
 */
void SettingsDialog::onActionChanged(QTreeWidgetItem *item, int column) {
    Q_UNUSED(item)
  if (column == 1 || column == 2) {
    readShortcuts();
  }
}
//---------------------------------------------------------------------------

/**
 * @brief Adds new custom action
 */
void SettingsDialog::addCustomAction() {
  actionsWidget->clearSelection();
  QTreeWidgetItem *tmp = new QTreeWidgetItem(actionsWidget,
                                             QStringList() << "*", 0);
  tmp->setFlags(Qt::ItemIsSelectable
                | Qt::ItemIsEditable
                | Qt::ItemIsEnabled
                | Qt::ItemIsDragEnabled
                | Qt::ItemIsUserCheckable);
  tmp->setCheckState(3, Qt::Unchecked);
  tmp->setSelected(true);
  actionsWidget->setCurrentItem(tmp);
  actionsWidget->scrollToItem(tmp);
  readShortcuts();
}
//---------------------------------------------------------------------------

/**
 * @brief Deletes custom action
 */
void SettingsDialog::delCustomAction() {
  delete actionsWidget->currentItem();
  readShortcuts();
}
//---------------------------------------------------------------------------

/**
 * @brief Displays info about custom action
 */
void SettingsDialog::infoCustomAction() {

  // Info
  QString info = tr("Use 'folder' to match all folders.<br>" \
                    "Use a folder name to match a specific folder.<br>" \
                    "Set text to 'Open' to override xdg default." \
                    "<p>%f - selected files<br>" \
                    "%F - selected files with full path<br>" \
                    "%n - current filename</p>" \
                    "<p>[] - tick checkbox to monitor output and errors.</p>");

  // Displays info
  QMessageBox::question(this, tr("Usage"), info);
}
//---------------------------------------------------------------------------
