#include "aboutdialog.h"

#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

/**
 * @brief Creates about dialog
 * @param parent
 */
AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {

  QString authors = "<strong>5. 2018 -</strong><br/> Ole-André Rodlie ("
                    "<a href=\"mailto:ole.andre.rodlie@gmail.com\">"
                    "ole.andre.rodlie@gmail.com</a>)<br/><br/>"
                    "<strong>8. 2012 - 5. 2013</strong><br/> Michal Rost ("
                    "<a href=\"mailto:rost.michal@gmail.com\">"
                    "rost.michal@gmail.com</a>)<br/><br/>"
                    "<strong>5. 2010 - 8. 2012</strong><br/> Wittfella ("
                    "<a href=\"mailto:wittfella@qtfm.org\">"
                    "wittfella@qtfm.org</a>)";
  QString thanks = "Eugene Pivnev - TI_Eugene ("
                   "<a href=\"mailto:ti.eugene@gmail.com\">ti.eugene@gmail.com"
                   "</a>)";

  this->setMinimumWidth(400);
  this->setMinimumHeight(350);
  this->setWindowTitle(tr("About %1").arg(APP_NAME));

  QVBoxLayout* layout = new QVBoxLayout(this);
  QTabWidget* tabWidget = new QTabWidget(this);
  QLabel* info = new QLabel(this);
  info->setText(QString("<h1>%1</h1><p>version: %2<br/><a href=\"http://qtfm.dracolinux.org\">qtfm.dracolinux.org</a></p>").arg(APP_NAME).arg(APP_VERSION));

  QLabel *icon = new QLabel(this);
  icon->setPixmap(QIcon::fromTheme("folder").pixmap(QSize(64, 64)));

  QWidget *header = new QWidget(this);
  header->setContentsMargins(0,0,0,0);
  QHBoxLayout *headerLayout = new QHBoxLayout(header);
  headerLayout->setMargin(0);
  headerLayout->setSpacing(0);
  headerLayout->addWidget(info);
  headerLayout->addStretch();
  headerLayout->addWidget(icon);

  QWidget* authorsTab = new QWidget(tabWidget);
  tabWidget->addTab(authorsTab, tr("Authors"));
  QGridLayout* authorsLayout = new QGridLayout(authorsTab);
  QLabel* lblAuthors = new QLabel(authors, authorsTab);
  lblAuthors->setAlignment(Qt::AlignHCenter);
  authorsLayout->addWidget(lblAuthors, 0, 1);
  authorsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 0);
  authorsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 2);

  QWidget* thanksTab = new QWidget(tabWidget);
  tabWidget->addTab(thanksTab, tr("Thanks"));
  QGridLayout* thanksLayout = new QGridLayout(thanksTab);
  QLabel* lblThanks = new QLabel(thanks, thanksTab);
  lblThanks->setAlignment(Qt::AlignHCenter);
  thanksLayout->addWidget(lblThanks, 0, 1);
  thanksLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 0);
  thanksLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 2);

  QHBoxLayout* btnLayout = new QHBoxLayout();
  QPushButton* btnClose = new QPushButton(tr("Close"), this);
  btnLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
  btnLayout->addWidget(btnClose);

  layout->addWidget(header);
  layout->addWidget(tabWidget);
  layout->addLayout(btnLayout);

  connect(btnClose, SIGNAL(clicked()), SLOT(close()));
}
//---------------------------------------------------------------------------
