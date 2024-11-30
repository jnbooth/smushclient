#pragma once
#include <QtCore/QDataStream>
#include "bridge/document.h"
#include "bridge/timekeeper.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

class SmushClient : public SmushClientBase
{
  Q_OBJECT

public:
  class VariableView
  {
  public:
    constexpr VariableView(const char *data, size_t size) noexcept : data_(data), size_(size) {}

    constexpr const char *data() const noexcept { return data_; }
    constexpr size_t size() const noexcept { return size_; };

    constexpr operator std::string_view() const noexcept { return std::string_view(data_, size_); }
    constexpr operator QByteArrayView() const noexcept { return QByteArrayView(data_, size_); }
    constexpr explicit operator std::string() const { return std::string(data_, size_); }
    inline explicit operator QByteArray() const { return QByteArray::fromRawData(data_, size_); }

  private:
    const char *data_;
    size_t size_;
  };

public:
  explicit SmushClient(QObject *parent = nullptr);

  VariableView getVariable(size_t index, std::string_view key) const noexcept;
  VariableView getMetavariable(std::string_view key) const noexcept;
  bool setVariable(size_t index, std::string_view key, std::string_view value) noexcept;
  bool setVariable(size_t index, std::string_view key, const QByteArray &value) noexcept;
  bool setMetavariable(std::string_view key, std::string_view value) noexcept;
  bool setMetavariable(std::string_view key, const QByteArray &value) noexcept;
};
