#pragma once
#include "bridge/document.h"
#include "bridge/timekeeper.h"
#include "bridge/variableview.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QDataStream>

inline rust::Slice<const uint8_t>
byteSlice(const uint8_t* data, size_t size) noexcept
{
  return rust::Slice(data, size);
}

inline rust::Slice<const uint8_t>
byteSlice(const char* data, size_t size) noexcept
{
  return rust::Slice(reinterpret_cast<const uint8_t*>(data), size);
}

template<typename C,
         std::enable_if_t<std::is_trivially_copyable_v<C>, bool> = true>
inline rust::Slice<const uint8_t>
byteSlice(C c) noexcept
{
  return byteSlice(c.data(), c.size());
}

template<typename C,
         std::enable_if_t<!std::is_trivially_copyable_v<C>, bool> = true>
inline rust::Slice<const uint8_t>
byteSlice(const C& c) noexcept
{
  return byteSlice(c.data(), c.size());
}

class SmushClient : public SmushClientBase
{
  Q_OBJECT

public:
  explicit inline SmushClient(QObject* parent = nullptr)
    : SmushClientBase(parent)
  {
  }

  inline VariableView getVariable(size_t index,
                                  std::string_view key) const noexcept
  {
    return SmushClientBase::getVariable(index, byteSlice(key));
  }

  inline VariableView getMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::getMetavariable(byteSlice(key));
  }

  inline bool hasMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::hasMetavariable(byteSlice(key));
  }

  inline bool setVariable(size_t index,
                          std::string_view key,
                          std::string_view value) const noexcept
  {
    return SmushClientBase::setVariable(
      index, byteSlice(key), byteSlice(value));
  }
  inline bool setVariable(size_t index,
                          std::string_view key,
                          const QByteArray& value) const noexcept
  {
    return SmushClientBase::setVariable(
      index, byteSlice(key), byteSlice(value));
  }

  inline bool setMetavariable(std::string_view key,
                              std::string_view value) const noexcept
  {
    return SmushClientBase::setMetavariable(byteSlice(key), byteSlice(value));
  }
  inline bool setMetavariable(std::string_view key,
                              const QByteArray& value) const noexcept
  {
    return SmushClientBase::setMetavariable(byteSlice(key), byteSlice(value));
  }

  inline bool unsetVariable(size_t index, std::string_view key) const noexcept
  {
    return SmushClientBase::unsetVariable(index, byteSlice(key));
  }

  inline bool unsetMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::unsetMetavariable(byteSlice(key));
  }
};
