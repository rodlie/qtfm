#include "processdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

/**
 * @brief Creates dialog and connects it to process
 * @param proc process
 * @param procName name of program that is running in process
 * @param parent parent widget
 */
ProcessDialog::ProcessDialog(QProcess* proc, const QString &procName,
                             QWidget *parent) : QDialog(parent) {

  // Saves process
  this->procPtr = proc;

  // Size
  this->setMinimumSize(420, 320);
  this->setWindowTitle(tr("Custom Action"));

  // Text
  QString text = tr("<h2>Custom action '%1' is running ...</h2>");

  // Create widgets
  output = new QListWidget(this);

  QWidget *labelContainer = new QWidget(this);
  QHBoxLayout *labelLayout = new QHBoxLayout(labelContainer);

  QLabel *labelIcon = new QLabel(this);
  labelIcon->setPixmap(QIcon::fromTheme("applications-system").pixmap(QSize(48, 48)));
  labelIcon->setMinimumSize(QSize(48, 48));
  labelIcon->setMaximumSize(QSize(48, 48));
  QLabel* labelText = new QLabel(text.arg(procName), this);
  labelLayout->addWidget(labelIcon);
  labelLayout->addWidget(labelText);

  QPushButton* btnClear = new QPushButton(tr("Clear"), this);
  QPushButton* btnClose = new QPushButton(tr("Hide"), this);
  QPushButton* btnTerminate = new QPushButton(tr("Abort"), this);

  // Create buttons layout
  QHBoxLayout* layoutBtns = new QHBoxLayout();
  layoutBtns->setSpacing(10);
  layoutBtns->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
  layoutBtns->addWidget(btnClear);
  layoutBtns->addWidget(btnClose);
  layoutBtns->addWidget(btnTerminate);

  // Label properties
  labelText->setWordWrap(true);

  // Create main layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(labelContainer);
  layout->addItem(new QSpacerItem(0, 10));
  //layout->addWidget(new QLabel(tr("Output:"), this));
  layout->addWidget(output);
  layout->addItem(layoutBtns);

  // Connect to process
  connect(procPtr, SIGNAL(started()), SLOT(onProcStarted()));
  connect(procPtr, SIGNAL(finished(int)), SLOT(onProcFinished()));
  connect(procPtr, SIGNAL(readyReadStandardOutput()), SLOT(onProcStdOut()));
  //connect(procPtr, SIGNAL(readyRead()), SLOT(onProcStdOut()));
  connect(btnTerminate, SIGNAL(clicked()), procPtr, SLOT(terminate()));
  connect(btnClear, SIGNAL(clicked()), output, SLOT(clear()));
  connect(btnClose, SIGNAL(clicked()), SLOT(hide()));
}
//---------------------------------------------------------------------------

/**
 * @brief Reaction on process finish
 */
void ProcessDialog::onProcFinished() {
  //this->hide();
    //qDebug() << "proc finished";
  this->deleteLater();
}
//---------------------------------------------------------------------------

/**
 * @brief Reaction on process start
 */
void ProcessDialog::onProcStarted() {
    //qDebug() << "proc started";
  this->show();
}
//---------------------------------------------------------------------------

/**
 * @brief Reaction on process standard output
 */
void ProcessDialog::onProcStdOut() {
  QProcess* p = qobject_cast<QProcess*>(sender());
  QString text = QString::fromLocal8Bit(p->readAllStandardOutput());
  output->addItem(new QListWidgetItem(text, output));
  output->scrollToBottom();
}
//---------------------------------------------------------------------------
