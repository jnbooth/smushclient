#include "timer.h"
#include "../client.h"
#include "../ui/worlddetails/timeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"
#include "smushclient_qt/src/ffi/timekeeper.cxxqt.h"

// Public methods

TimerModel::TimerModel(SmushClient& client,
                       Timekeeper* timekeeper,
                       QObject* parent)
  : AbstractSenderModel(client, SenderType::Timer, parent)
  , timekeeper(timekeeper)
{
  client.stopTimers();
}

// Public overrides

QString
TimerModel::exportXml() const
{
  return client.exportWorldTimers();
}

Qt::ItemFlags
TimerModel::flags(const QModelIndex& index) const
{
  if (!index.constInternalPointer()) {
    return Qt::ItemFlag::ItemIsEnabled;
  }

  const Qt::ItemFlags flags = Qt::ItemFlag::ItemIsSelectable |
                              Qt::ItemFlag::ItemIsEnabled |
                              Qt::ItemFlag::ItemNeverHasChildren;

  const int column = index.column();
  return column == 0 || column == 3 ? flags | Qt::ItemFlag::ItemIsEditable
                                    : flags;
}

// Protected overrides

bool
TimerModel::add(QWidget* parent)
{
  Timer timer;
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected) {
    return false;
  }
  return client.addWorldTimer(timer, *timekeeper) == ApiCode::OK;
}

int
TimerModel::edit(size_t index, QWidget* parent)
{
  Timer timer(client, index);
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected) {
    return (int)ReplaceSenderResult::Unchanged;
  }
  return client.replaceWorldTimer(index, timer, *timekeeper);
}

const std::array<QString, 4>&
TimerModel::headers() const noexcept
{
  const static std::array<QString, 4> headers{
    tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text")
  };
  return headers;
}

RegexParse
TimerModel::import(const QString& xml)
{
  return client.importWorldTimers(xml, *timekeeper);
}

void
TimerModel::prepareRemove(SenderMap* senderMap,
                          const rust::String& group,
                          int row,
                          int count)
{
  timekeeper->cancelTimers(senderMap->timerIds(client, group, row, count));
}
