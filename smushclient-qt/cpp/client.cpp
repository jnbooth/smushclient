#include "client.h"

using std::string_view;

// Private utils

inline rust::slice<const char> byteSlice(const QByteArray &bytes) noexcept
{
  return rust::slice<const char>(bytes.data(), bytes.size());
}

inline rust::slice<const char> stringSlice(string_view view) noexcept
{
  return rust::slice<const char>(view.data(), view.size());
}

// Public methods

SmushClient::SmushClient(QObject *parent)
    : SmushClientBase(parent) {}

VariableView SmushClient::getVariable(size_t index, string_view key) const noexcept
{
  size_t size;
  const char *data = SmushClientBase::getVariable(index, stringSlice(key), &size);
  return VariableView(data, size);
}

VariableView SmushClient::getMetavariable(string_view key) const noexcept
{
  size_t size;
  const char *data = SmushClientBase::getMetavariable(stringSlice(key), &size);
  return VariableView(data, size);
}

bool SmushClient::setVariable(size_t index, string_view key, string_view value) noexcept
{
  return SmushClientBase::setVariable(index, stringSlice(key), stringSlice(value));
}

bool SmushClient::setVariable(size_t index, string_view key, const QByteArray &value) noexcept
{
  return SmushClientBase::setVariable(index, stringSlice(key), byteSlice(value));
}

bool SmushClient::setMetavariable(string_view key, string_view value) noexcept
{
  return SmushClientBase::setMetavariable(stringSlice(key), stringSlice(value));
}

bool SmushClient::setMetavariable(string_view key, const QByteArray &value) noexcept
{
  return SmushClientBase::setMetavariable(stringSlice(key), byteSlice(value));
}

bool SmushClient::unsetVariable(size_t index, string_view key) noexcept
{
  return SmushClientBase::unsetVariable(index, stringSlice(key));
}

bool SmushClient::unsetMetavariable(string_view key) noexcept
{
  return SmushClientBase::unsetMetavariable(stringSlice(key));
}
