#include "mudstatusbar.h"
#include "statusbarstat.h"
#include "ui_mudstatusbar.h"
#include <QtCore/QDataStream>
#include <QtCore/QIODevice>
#include <QtCore/QSettings>

// Private utils

// Public methods

MudStatusBar::MudStatusBar(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::MudStatusBar)
  , connectionIcons({
      QIcon(QStringLiteral(":/icons/status/disconnected.svg")),
      QIcon(QStringLiteral(":/icons/status/connected.svg")),
      QIcon(QStringLiteral(":/icons/status/encrypted.svg")),
    })
{
  ui->setupUi(this);
  menu = new QMenu(this);
  menu->close();
  menu->addAction(ui->action_connection_status);
  menu->addAction(ui->action_users_online);
  ui->connection->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui->users->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui->connection->setVisible(false);
  ui->users->setVisible(false);
  setConnected(ConnectionStatus::Disconnected);
  restore();
}

MudStatusBar::~MudStatusBar()
{
  save();
  delete ui;
}

bool
MudStatusBar::createStat(const QString& entity,
                         const QString& caption,
                         const QString& maxEntity)
{
  auto search = statsByEntity.find(entity);
  if (search != statsByEntity.end()) {
    return recreateStat(search.value(), caption, maxEntity);
  }

  StatusBarStat* stat = new StatusBarStat(entity, caption, maxEntity, this);
  ui->horizontalLayout->insertWidget(static_cast<int>(statsByEntity.size()),
                                     stat);
  statsByEntity[entity] = stat;
  if (!maxEntity.isEmpty()) {
    statsByMax.insert(maxEntity, stat);
  }
  QMenu* statMenu = stat->menu();
  menu->addMenu(statMenu);
  statMenu->menuAction()->setVisible(false);

  return true;
}

bool
MudStatusBar::updateStat(const QString& entity, const QString& value)
{
  for (auto iter = statsByMax.find(entity), end = statsByMax.end();
       iter != end && iter.key() == entity;
       ++iter) {
    iter.value()->setMax(value);
  }

  auto search = statsByEntity.find(entity);
  if (search == statsByEntity.end()) [[unlikely]] {
    return false;
  }

  StatusBarStat* stat = search.value();
  stat->setValue(value);
  return true;
}

QString
MudStatusBar::message() const
{
  return ui->message->text();
}

// Public slots

void
MudStatusBar::clearStats()
{
  qDeleteAll(statsByEntity.values());
  statsByEntity.clear();
  statsByMax.clear();
}

void
MudStatusBar::setConnected(MudStatusBar::ConnectionStatus status)
{
  const bool connected = status != ConnectionStatus::Disconnected;
  ui->connection->setText(status == ConnectionStatus::Disconnected
                            ? tr("Disconnected")
                            : tr("Connected"));
  ui->connection->setIcon(connectionIcons.at(static_cast<size_t>(status)));

  if (connected) {
    return;
  }

  ui->users->hide();
  clearStats();
}

void
MudStatusBar::setMessage(const QString& message)
{
  ui->message->setText(message);
}

void
MudStatusBar::setUsers(int users)
{
  setUsers(QString::number(users));
}

void
MudStatusBar::setUsers(const QString& users)
{
  ui->users->setText(users);
  ui->users->setVisible(ui->action_users_online->isChecked());
}

// Protected overrides

void
MudStatusBar::contextMenuEvent(QContextMenuEvent* event)
{
  menu->popup(event->globalPos());
}

// Private methods

bool
MudStatusBar::recreateStat(StatusBarStat* stat,
                           const QString& caption,
                           const QString& maxEntity)
{
  stat->setCaption(caption);
  const QString& currentMaxEntity = stat->maxEntity();
  if (currentMaxEntity == maxEntity) {
    return false;
  }

  statsByMax.remove(currentMaxEntity, stat);
  stat->setMaxEntity(maxEntity);
  statsByMax.insert(maxEntity, stat);
  return true;
}

bool
MudStatusBar::restore()
{
  const QByteArray saveData = QSettings().value(settingsKey()).toByteArray();
  if (saveData.isEmpty()) {
    for (QAction* action : stateActions()) {
      action->setChecked(true);
    }

    return false;
  }

  bool check;
  QDataStream stream(saveData);
  for (QAction* action : stateActions()) {
    stream >> check;
    action->setChecked(check);
  }

  return stream.status() == QDataStream::Status::Ok;
}

void
MudStatusBar::save() const
{
  QByteArray saveData;
  QDataStream stream(&saveData, QIODevice::WriteOnly);
  for (QAction* action : stateActions()) {
    stream << action->isChecked();
  }
  QSettings().setValue(settingsKey(), saveData);
}

const QString&
MudStatusBar::settingsKey()
{
  static const QString key = QStringLiteral("state/stat");
  return key;
}

QList<QAction*>
MudStatusBar::stateActions() const
{
  return { ui->action_connection_status, ui->action_users_online };
}
