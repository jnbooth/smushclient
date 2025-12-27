#pragma once
#include "bridge/document.h"
#include "bridge/timekeeper.h"
#include "bridge/variableview.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include <QtCore/QDataStream>

class SmushClient : public SmushClientBase {
  Q_OBJECT

public:
  explicit inline SmushClient(QObject *parent = nullptr)
      : SmushClientBase(parent) {}

  inline VariableView getVariable(size_t index,
                                  std::string_view key) const noexcept {
    return SmushClientBase::getVariable(index, rust::Slice(key));
  }

  inline VariableView getMetavariable(std::string_view key) const noexcept {
    return SmushClientBase::getMetavariable(rust::Slice(key));
  }

  inline bool hasMetavariable(std::string_view key) const noexcept {
    return SmushClientBase::hasMetavariable(rust::Slice(key));
  }

  inline bool setVariable(size_t index, std::string_view key,
                          std::string_view value) const noexcept {
    return SmushClientBase::setVariable(index, rust::Slice(key),
                                        rust::Slice(value));
  }
  inline bool setVariable(size_t index, std::string_view key,
                          const QByteArray &value) const noexcept {
    return SmushClientBase::setVariable(index, rust::Slice(key),
                                        rust::Slice(value));
  }

  inline bool setMetavariable(std::string_view key,
                              std::string_view value) const noexcept {
    return SmushClientBase::setMetavariable(rust::Slice(key),
                                            rust::Slice(value));
  }
  inline bool setMetavariable(std::string_view key,
                              const QByteArray &value) const noexcept {
    return SmushClientBase::setMetavariable(rust::Slice(key),
                                            rust::Slice(value));
  }

  inline bool unsetVariable(size_t index, std::string_view key) const noexcept {
    return SmushClientBase::unsetVariable(index, rust::Slice(key));
  }

  inline bool unsetMetavariable(std::string_view key) const noexcept {
    return SmushClientBase::unsetMetavariable(rust::Slice(key));
  }
};
