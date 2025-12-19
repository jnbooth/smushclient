#pragma once
#include "bridge/document.h"
#include "bridge/timekeeper.h"
#include "smushclient_qt/src/ffi/client.cxxqt.h"
#include "variableview.h"
#include <QtCore/QDataStream>

class SmushClient : public SmushClientBase {
  Q_OBJECT

public:
  explicit SmushClient(QObject *parent = nullptr);

  VariableView getVariable(size_t index, std::string_view key) const noexcept;
  VariableView getMetavariable(std::string_view key) const noexcept;
  bool hasMetavariable(std::string_view key) const noexcept;
  bool setVariable(size_t index, std::string_view key,
                   std::string_view value) const noexcept;
  bool setVariable(size_t index, std::string_view key,
                   const QByteArray &value) const noexcept;
  bool setMetavariable(std::string_view key,
                       std::string_view value) const noexcept;
  bool setMetavariable(std::string_view key,
                       const QByteArray &value) const noexcept;
  bool unsetVariable(size_t index, std::string_view key) const noexcept;
  bool unsetMetavariable(std::string_view key) const noexcept;
};
