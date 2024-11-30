#include "mudstatusbar.h"
#include "ui_mudstatusbar.h"
#include "statusbarstat.h"
#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

// Public methods

MudStatusBar::MudStatusBar(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MudStatusBar)
{
  ui->setupUi(this);
  addAction(ui->action_connection_status);
  addAction(ui->action_users_online);
  ui->connection->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui->users->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui->connection->setVisible(false);
  ui->users->setVisible(false);
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
    QAction *action = stat->action();
    addAction(action);
    if (hiddenEntities.contains(entity))
      action->setChecked(false);
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

bool MudStatusBar::restore(const QByteArray &data)
{
  hiddenEntities.clear();
  if (data.isEmpty())
  {
    ui->action_connection_status->setChecked(true);
    ui->action_users_online->setChecked(true);
    return true;
  }
  QDataStream stream(data);
  bool showConnection;
  bool showUsers;
  stream >> showConnection >> showUsers >> hiddenEntities;
  ui->action_connection_status->setChecked(showConnection);
  ui->action_users_online->setChecked(showUsers);

  return stream.status() == QDataStream::Status::Ok;
}

QByteArray MudStatusBar::save() const
{
  QSet<QString> entities = hiddenEntities;
  for (auto iter = statsByEntity.cbegin(), end = statsByEntity.cend(); iter != end; ++iter)
  {
    if (iter.value()->isVisible())
      entities.remove(iter.key());
    else
      entities.insert(iter.key());
  }
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << ui->action_connection_status->isChecked() << ui->action_users_online->isChecked() << entities;
  return data;
}

bool MudStatusBar::updateStat(const QString &entity, const QString &value)
{
  for (auto iter = statsByMax.find(entity), end = statsByMax.end(); iter != end && iter.key() == entity; ++iter)
    iter.value()->setMax(value);

  auto search = statsByEntity.find(entity);
  if (search == statsByEntity.end()) [[unlikely]]
    return false;

  StatusBarStat *stat = search.value();
  QAction *action = stat->action();
  stat->setValue(value);
  if (value.isEmpty())
  {
    action->setVisible(false);
    stat->setVisible(false);
    return true;
  }
  action->setVisible(true);
  stat->setVisible(action->isChecked());
  return true;
}

// Public slots

void MudStatusBar::clearStats()
{
  for (auto iter = statsByEntity.cbegin(), end = statsByEntity.cend(); iter != end; ++iter)
  {
    if (iter.value()->action()->isChecked())
      hiddenEntities.remove(iter.key());
    else
      hiddenEntities.insert(iter.key());
  }
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
