#include "client.h"
#include "bridge/timekeeper.h"
#include <QtCore/QDataStream>

using std::string_view;

// Public methods

bool
SmushClient::finishTimer(const Timekeeper::Item& item)
{
  return SmushClientBase::finishTimer(item.index);
}

// Public slots

void
SmushClient::onTimersPolled() noexcept
{
  pollTimers();
}
