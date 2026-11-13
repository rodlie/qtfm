#include "applicationdialog.h"
#include "fileutils.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCompleter>
#include <QApplication>
#include <QStringListModel>
#include <QTimer>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

/**
 * @brief Constructor
 * @param parent
 */

ApplicationDialog::ApplicationDialog(bool enable_launcher,
                                     QWidget *parent) :
    QDialog(parent)
  , appList(nullptr)
  , edtCommand(nullptr)
  , edtCustom(nullptr)
  , chkTerminal(nullptr)
  , chkDefault(nullptr)
{
    // Title and size
    setWindowTitle(tr("Select application"));
    setMinimumSize(320, 320);

    // Creates app list view
    appList = new QTreeWidget(this);
    appList->setIconSize(QSize(24, 24));
    appList->setAlternatingRowColors(true);
    appList->headerItem()->setText(0, tr("Application"));

    // Creates buttons
    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    // Command bar
    edtCommand = new QLineEdit(this);
    edtCommand->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFormLayout* layoutCommand = new QFormLayout();
    if (enable_launcher) {
        layoutCommand->addRow(tr("Launcher: "), edtCommand);
    }

    // Custom command: an application that is not in the list
    QGroupBox *customGroup = new QGroupBox(tr("Custom command"), this);
    edtCustom = new QLineEdit(customGroup);
    edtCustom->setPlaceholderText(tr("Program or command line, e.g. /opt/app/bin/app %f"));
    edtCustom->setClearButtonEnabled(true);
    QPushButton *btnBrowse = new QPushButton(tr("Browse..."), customGroup);
    chkTerminal = new QCheckBox(tr("Run in terminal"), customGroup);
    QHBoxLayout *customLine = new QHBoxLayout();
    customLine->addWidget(edtCustom, 1);
    customLine->addWidget(btnBrowse);
    QVBoxLayout *customLayout = new QVBoxLayout(customGroup);
    customLayout->addLayout(customLine);
    customLayout->addWidget(chkTerminal);
    connect(btnBrowse, SIGNAL(clicked()), this, SLOT(browseCommand()));
    connect(edtCustom, SIGNAL(textEdited(QString)), this, SLOT(customCommandEdited(QString)));

    // Use as default (shown by setMimeType())
    chkDefault = new QCheckBox(this);
    chkDefault->hide();

    // Layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(appList);
    layout->addLayout(layoutCommand);
    layout->addWidget(customGroup);
    layout->addWidget(chkDefault);
    layout->addWidget(buttons);

    // Synonyms for category names
    catNames.clear();
    catNames.insert("Development", QStringList() << "Programming");
    catNames.insert("Games", QStringList() << "Game");
    catNames.insert("Graphics", QStringList());
    catNames.insert("Internet", QStringList() << "Network" << "WebBrowser");
    catNames.insert("Multimedia", QStringList() << "AudioVideo" << "Video");
    catNames.insert("Office", QStringList());
    catNames.insert("Other", QStringList());
    catNames.insert("Settings", QStringList() << "System");
    catNames.insert("Utilities", QStringList() << "Utility");

    // Load default icon
    defaultIcon = QIcon::fromTheme("application-x-executable");

    // Create completer and its model for editation of command
    QStringListModel* model = new QStringListModel(this);
    model->setStringList(applications.keys());
    QCompleter* completer = new QCompleter(this);
    completer->setModel(model);
    edtCommand->setCompleter(completer);
    if (!enable_launcher) { edtCommand->hide(); }

    // Signals
    connect(appList,
           SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(updateCommand(QTreeWidgetItem*,QTreeWidgetItem*)));

    // populate
    QTimer::singleShot(100, this, SLOT(populate()));
}

/**
 * @brief Returns currently selected launcher
 * @return currently selected launcher
 */

QString ApplicationDialog::getCurrentLauncher() const
{
    if (!chosenLauncher.isEmpty()) { return chosenLauncher; }
    return edtCommand->text();
}

void ApplicationDialog::setMimeType(const QString &mime, bool offerDefault)
{
    mimeType = mime;
    chkDefault->setText(tr("Use as default for %1 files").arg(mime));
    chkDefault->setVisible(offerDefault && !mime.isEmpty());
}

bool ApplicationDialog::useAsDefault() const
{
    return !chkDefault->isHidden() && chkDefault->isChecked();
}

void ApplicationDialog::browseCommand()
{
    QString start = QFileInfo(QProcess::splitCommand(edtCustom->text()).value(0)).absolutePath();
    if (edtCustom->text().isEmpty() || !QFileInfo(start).isDir()) { start = "/usr/bin"; }
    const QString file = QFileDialog::getOpenFileName(this, tr("Select program"), start);
    if (file.isEmpty()) { return; }
    // quote paths with spaces, they are split like a command line
    edtCustom->setText(file.contains(' ') ? QString("\"%1\"").arg(file) : file);
    customCommandEdited(edtCustom->text());
}

void ApplicationDialog::customCommandEdited(const QString &text)
{
    // a custom command replaces the application chosen in the list
    if (!text.isEmpty()) {
        appList->clearSelection();
        appList->setCurrentItem(nullptr);
        edtCommand->clear();
    }
}

void ApplicationDialog::accept()
{
    const QString command = edtCustom->text().trimmed();
    if (command.isEmpty()) {
        chosenLauncher.clear();
        QDialog::accept();
        return;
    }
    const QString name = FileUtils::createUserApplication(command, chkTerminal->isChecked(), mimeType);
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Custom command"),
                             tr("Could not create an application for \"%1\" in %2.")
                             .arg(command, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                                  + "/applications"));
        return;
    }
    chosenLauncher = name;
    QDialog::accept();
}

void ApplicationDialog::populate()
{
    // Create default application categories
    categories.clear();
    createCategories();

    // Load applications and create category tree list
    QList<DesktopFile> apps = FileUtils::getApplications();
    foreach (DesktopFile app, apps) {
        // Check for name
        if (app.getName().compare("") == 0 || app.noDisplay()) { continue; }

        // Find category
        QTreeWidgetItem* category = findCategory(app);

        // Create item from current mime
        QTreeWidgetItem *item = new QTreeWidgetItem(category);
        item->setIcon(0, FileUtils::searchAppIcon(app, defaultIcon));
        item->setText(0, app.getName());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        // Register application
        QApplication::processEvents();
        applications.insert(app.getPureFileName(), item);
    }
}

/**
 * @brief Creates default application categories
 * @param names names of categories with synonyms
 */

void ApplicationDialog::createCategories()
{
    // Create categories
    foreach (QString name, catNames.keys()) {
        // Find icon
        QIcon icon = QIcon::fromTheme("applications-" + name.toLower());

        // If icon not found, check synonyms
        if (icon.isNull()) {
            foreach (QString synonym, catNames.value(name)) {
                icon = QIcon::fromTheme("applications-" + synonym.toLower());
                break;
            }
        }

        // If icon still not found, retrieve default icon
        if (icon.isNull()) { icon = defaultIcon; }

        // Create category
        QTreeWidgetItem* category = new QTreeWidgetItem(appList);
        category->setText(0, name);
        category->setIcon(0, icon);
        category->setFlags(Qt::ItemIsEnabled);
        categories.insert(name, category);
    }
}

/**
 * @brief Searches the most suitable category for application
 * @param app
 * @return category
 */

QTreeWidgetItem* ApplicationDialog::findCategory(const DesktopFile &app)
{
    // Default category is 'Other'
    QTreeWidgetItem* category = categories.value("Other");

    // Try to find more suitable category
    foreach (QString name, catNames.keys()) {
        // Try category name
        if (app.getCategories().contains(name)) {
            category = categories.value(name);
            break;
        }
        // Try synonyms
        bool found = false;
        foreach (QString synonym, catNames.value(name)) {
            if (app.getCategories().contains(synonym)) {
                found = true;
                break;
            }
        }
        if (found) {
            category = categories.value(name);
            break;
        }
    }
    return category;
}

/**
 * @brief Updates launcher command
 * @param current
 * @param previous
 */

void ApplicationDialog::updateCommand(QTreeWidgetItem *current,
                                      QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    edtCommand->setText(applications.key(current));
    // choosing an application in the list replaces a custom command
    if (current && !applications.key(current).isEmpty()) { edtCustom->clear(); }
}
