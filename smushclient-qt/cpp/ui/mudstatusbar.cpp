#include "mudstatusbar.h"
#include "ui_mudstatusbar.h"
#include "statusbarstat.h"

// Public methods

MudStatusBar::MudStatusBar(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MudStatusBar),
      statsByEntity(),
      statsByMax()
{
  ui->setupUi(this);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setConnected(false);
}

MudStatusBar::~MudStatusBar()
{
  delete ui;
}

bool MudStatusBar::createStat(const QString &entity, const QString &caption, const QString &max)
{
  auto search = statsByEntity.find(entity);
  if (search == statsByEntity.end())
  {
    StatusBarStat *stat = new StatusBarStat(caption, max, this);
    layout()->addWidget(stat);
    statsByEntity[entity] = stat;
    if (!max.isEmpty())
      statsByMax.insert(max, stat);
    return true;
  }
  StatusBarStat *stat = search.value();
  stat->setCaption(caption);
  const QString &currentMaxEntity = stat->maxEntity();
  if (currentMaxEntity == max)
    return false;

  statsByMax.remove(currentMaxEntity, stat);
  stat->setMaxEntity(max);
  statsByMax.insert(max, stat);

  return false;
}

bool MudStatusBar::updateStat(const QString &entity, const QString &value)
{
  for (auto iter = statsByMax.find(entity), end = statsByMax.end(); iter != end && iter.key() == entity; ++iter)
    iter.value()->setMax(value);

  auto search = statsByEntity.find(entity);
  if (search == statsByEntity.end()) [[unlikely]]
    return false;

  search.value()->setValue(value);
  return true;
}

// Public slots

void MudStatusBar::clearStats()
{
  qDeleteAll(statsByEntity.values());
  statsByEntity.clear();
  statsByMax.clear();
}

void MudStatusBar::setConnected(bool connected)
{
  ui->connection->setEnabled(connected);
  ui->connection->setText(connected ? tr("Connected") : tr("Disconnected"));
  if (connected)
    return;

  ui->users->hide();
  clearStats();
}

void MudStatusBar::setMessage(const QString &message)
{
  ui->message->setText(message);
}

void MudStatusBar::setUsers(int users)
{
  setUsers(QString::number(users));
}

void MudStatusBar::setUsers(const QString &users)
{
  ui->users->setText(users);
  ui->users->show();
}
