#include "mudstatusbar.h"
#include "ui_mudstatusbar.h"

// Public methods

MudStatusBar::MudStatusBar(QWidget *parent)
    : QWidget(parent), ui(new Ui::MudStatusBar)
{
  ui->setupUi(this);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setConnected(false);
}

MudStatusBar::~MudStatusBar()
{
  delete ui;
}

// Public slots

void MudStatusBar::setConnected(bool connected)
{
  ui->connection->setEnabled(connected);
  ui->connection->setText(connected ? tr("Connected") : tr("Disconnected"));
  if (!connected)
    ui->users->hide();
}

void MudStatusBar::setMessage(const QString &message)
{
  ui->message->setText(message);
}

void MudStatusBar::setUsers(int users)
{
  ui->users->setText(QString::number(users));
  ui->users->show();
}
