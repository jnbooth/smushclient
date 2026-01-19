#pragma once
#include "bridge/variableview.h"
#include "bytes.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QDataStream>

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
    return SmushClientBase::getVariable(index, bytes::slice(key));
  }

  inline VariableView getMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::getMetavariable(bytes::slice(key));
  }

  inline bool hasMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::hasMetavariable(bytes::slice(key));
  }

  inline bool setVariable(size_t index,
                          std::string_view key,
                          std::string_view value) const noexcept
  {
    return SmushClientBase::setVariable(
      index, bytes::slice(key), bytes::slice(value));
  }
  inline bool setVariable(size_t index,
                          std::string_view key,
                          const QByteArray& value) const noexcept
  {
    return SmushClientBase::setVariable(
      index, bytes::slice(key), bytes::slice(value));
  }

  inline bool setMetavariable(std::string_view key,
                              std::string_view value) const noexcept
  {
    return SmushClientBase::setMetavariable(bytes::slice(key),
                                            bytes::slice(value));
  }
  inline bool setMetavariable(std::string_view key,
                              const QByteArray& value) const noexcept
  {
    return SmushClientBase::setMetavariable(bytes::slice(key),
                                            bytes::slice(value));
  }

  inline bool unsetVariable(size_t index, std::string_view key) const noexcept
  {
    return SmushClientBase::unsetVariable(index, bytes::slice(key));
  }

  inline bool unsetMetavariable(std::string_view key) const noexcept
  {
    return SmushClientBase::unsetMetavariable(bytes::slice(key));
  }
};
