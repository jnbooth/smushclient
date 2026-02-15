#include "timer.h"
#include "../client.h"
#include "../ui/worlddetails/timeredit.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include "smushclient_qt/src/ffi/sender_map.cxxqt.h"

using std::array;

// Public methods

TimerModel::TimerModel(SmushClient& client, QObject* parent)
  : AbstractSenderModel(client, SenderType::Timer, parent)
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
  if (hasChildren(index)) {
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
  return client.addWorldTimer(timer) == ApiCode::OK;
}

int
TimerModel::edit(size_t index, QWidget* parent)
{
  Timer timer(client, index);
  TimerEdit edit(timer, parent);
  if (edit.exec() == QDialog::Rejected) {
    return static_cast<int>(ReplaceSenderResult::Unchanged);
  }
  return client.replaceWorldTimer(index, timer);
}

const array<QString, 4>&
TimerModel::headers() const noexcept
{
  const static array<QString, 4> headers{
    tr("Group/Label"), tr("Type"), tr("Occurrence"), tr("Text")
  };
  return headers;
}

RegexParse
TimerModel::import(const QString& xml)
{
  return client.importWorldTimers(xml);
}

void
TimerModel::prepareRemove(SenderMap& senderMap,
                          const rust::String& group,
                          int row,
                          int count)
{
  client.cancelTimers(senderMap.timerIds(client, group, row, count));
}
