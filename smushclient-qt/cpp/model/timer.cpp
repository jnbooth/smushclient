#include "timer.h"
#include "../bridge/timekeeper.h"
#include "../client.h"
#include "../ui/worlddetails/timeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

using std::array;

// Public methods

TimerModel::TimerModel(SmushClient& client,
                       Timekeeper& timekeeper,
                       QObject* parent)
  : AbstractSenderModel(client, SenderKind::Timer, parent)
  , timekeeper(timekeeper)
{
}

// Public overrides

Qt::ItemFlags
TimerModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = AbstractSenderModel::flags(index);
  const int column = index.column();
  if (column == 0 || column == 3) {
    flags.setFlag(Qt::ItemFlag::ItemIsEditable, false);
  }
  return flags;
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
  return client.addWorldTimer(timer, timekeeper) == ApiCode::OK;
}

int
TimerModel::edit(size_t index, QWidget* parent)
{
  Timer timer(client, index);
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected) {
    return static_cast<int>(ReplaceSenderResult::Unchanged);
  }
  return client.replaceWorldTimer(index, timer, timekeeper);
}

QVariant
TimerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  static const array<QString, numColumns> headers{
    tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text")
  };

  if (orientation != Qt::Orientation::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  return headers.at(section);
}

ParseResult
TimerModel::import(const QString& xml)
{
  return client.importWorldTimers(xml, timekeeper);
}

void
TimerModel::prepareRemove(SenderMap& senderMap,
                          const rust::String& group,
                          int row,
                          int count)
{
  timekeeper.cancelTimers(senderMap.timerIds(client, group, row, count));
}
