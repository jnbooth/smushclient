#pragma once
#include <QtCore/QByteArray>

class VariableView
{
public:
  constexpr VariableView(const char *data, size_t size) noexcept : data_(data), size_(size) {}

  constexpr const char *data() const noexcept { return data_; }
  constexpr size_t size() const noexcept { return size_; };

  constexpr operator bool() const noexcept { return data_; }
  constexpr operator std::string_view() const noexcept { return std::string_view(data_, size_); }
  constexpr operator QByteArrayView() const noexcept { return QByteArrayView(data_, size_); }
  constexpr explicit operator std::string() const { return std::string(data_, size_); }
  inline explicit operator QByteArray() const { return QByteArray::fromRawData(data_, size_); }

private:
  const char *data_;
  size_t size_;
};
