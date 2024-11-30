#pragma once
#include <QtCore/QDataStream>
#include "bridge/document.h"
#include "bridge/timekeeper.h"
#include "variableview.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

class SmushClient : public SmushClientBase
{
  Q_OBJECT

public:
  explicit SmushClient(QObject *parent = nullptr);

  VariableView getVariable(size_t index, std::string_view key) const noexcept;
  VariableView getMetavariable(std::string_view key) const noexcept;
  bool setVariable(size_t index, std::string_view key, std::string_view value) noexcept;
  bool setVariable(size_t index, std::string_view key, const QByteArray &value) noexcept;
  bool setMetavariable(std::string_view key, std::string_view value) noexcept;
  bool setMetavariable(std::string_view key, const QByteArray &value) noexcept;
};
