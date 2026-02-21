// NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
#ifndef VIEWS_H
#define VIEWS_H
#include <QtCore/QByteArray>

#define CAST(V, T)                                                             \
  V(T view) noexcept                                                           \
    : data_(view.data())                                                       \
    , size_(view.size())                                                       \
  {                                                                            \
  }

namespace rust {
class bytes_view
{
public:
  constexpr bytes_view(const char* data, size_t size) noexcept
    : data_(data)
    , size_(size)
  {
  }

  constexpr CAST(bytes_view, const std::string&);
  constexpr CAST(bytes_view, std::string_view);
  constexpr CAST(bytes_view, QByteArrayView);
  CAST(bytes_view, const QByteArray&);

  constexpr bytes_view(const char* cstr) noexcept
    : data_(cstr)
    , size_(std::char_traits<char>::length(cstr))
  {
  }

  constexpr const char* data() const noexcept { return data_; }
  constexpr size_t size() const noexcept { return size_; };

  // DO NOT EDIT THESE FIELDS
  // They must be consistent with smushclient-qt/rust/src/ffi/bytes_view.rs
private:
  const char* data_;
  size_t size_;
};

class string_view
{
public:
  constexpr string_view(const char* data, size_t size) noexcept
    : data_(data)
    , size_(size)
  {
  }

  constexpr CAST(string_view, std::string_view);
  constexpr CAST(string_view, const std::string&);

  constexpr string_view(const char* cstr) noexcept
    : data_(cstr)
    , size_(std::char_traits<char>::length(cstr))
  {
  }

  constexpr const char* data() const noexcept { return data_; }
  constexpr size_t size() const noexcept { return size_; };

  // DO NOT EDIT THESE FIELDS
  // They must be consistent with smushclient-qt/rust/src/ffi/string_view.rs
private:
  const char* data_;
  size_t size_;
};

class variable_view
{
public:
  constexpr variable_view(const char* data, size_t size) noexcept
    : data_(data)
    , size_(size)
  {
  }

  constexpr const char* data() const noexcept { return data_; }
  constexpr size_t size() const noexcept { return size_; };

  constexpr operator bool() const noexcept { return data_ != nullptr; }
  constexpr operator std::string_view() const noexcept
  {
    return std::string_view(data_, size_);
  }
  constexpr operator QByteArrayView() const noexcept
  {
    return QByteArrayView(data_, static_cast<qsizetype>(size_));
  }
  explicit operator std::string() const { return std::string(data_, size_); }
  explicit operator QByteArray() const
  {
    return QByteArray::fromRawData(data_, static_cast<qsizetype>(size_));
  }

  // DO NOT EDIT THESE FIELDS
  // They must be consistent with smushclient-qt/rust/src/ffi/variable_views.rs
private:
  const char* data_;
  size_t size_;
};
} // namespace rust

#undef CAST
#endif // VIEWS_H
// NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)
