#include "client.h"
#include "bridge/timekeeper.h"
#include "bridge/variableview.h"
#include "bytes.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QDataStream>

using std::string_view;

// Public methods

SmushClient::SmushClient(QObject* parent)
  : SmushClientBase(parent)
{
}

VariableView
SmushClient::getVariable(size_t index, string_view key) const noexcept
{
  return SmushClientBase::getVariable(index, bytes::slice(key));
}

VariableView
SmushClient::getMetavariable(string_view key) const noexcept
{
  return SmushClientBase::getMetavariable(bytes::slice(key));
}

bool
SmushClient::hasMetavariable(string_view key) const noexcept
{
  return SmushClientBase::hasMetavariable(bytes::slice(key));
}

bool
SmushClient::setVariable(size_t index,
                         string_view key,
                         string_view value) const noexcept
{
  return SmushClientBase::setVariable(
    index, bytes::slice(key), bytes::slice(value));
}

bool
SmushClient::setVariable(size_t index,
                         string_view key,
                         const QByteArray& value) const noexcept
{
  return SmushClientBase::setVariable(
    index, bytes::slice(key), bytes::slice(value));
}

bool
SmushClient::setMetavariable(string_view key, string_view value) const noexcept
{
  return SmushClientBase::setMetavariable(bytes::slice(key),
                                          bytes::slice(value));
}

bool
SmushClient::setMetavariable(string_view key,
                             const QByteArray& value) const noexcept
{
  return SmushClientBase::setMetavariable(bytes::slice(key),
                                          bytes::slice(value));
}

bool
SmushClient::finishTimer(const Timekeeper::Item& item)
{
  return SmushClientBase::finishTimer(item.index);
}

bool
SmushClient::unsetVariable(size_t index, string_view key) const noexcept
{
  return SmushClientBase::unsetVariable(index, bytes::slice(key));
}

bool
SmushClient::unsetMetavariable(string_view key) const noexcept
{
  return SmushClientBase::unsetMetavariable(bytes::slice(key));
}

// Public slots

void
SmushClient::onTimersPolled() noexcept
{
  pollTimers();
}
