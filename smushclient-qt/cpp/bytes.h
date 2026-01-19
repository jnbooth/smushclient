#pragma once
#include "rust/cxx.h"
#include <QtCore/QString>

namespace bytes {
inline rust::Slice<const uint8_t>
slice(const uint8_t* data, size_t size) noexcept
{
  return rust::Slice(data, size);
}

inline rust::Slice<const uint8_t>
slice(const char* data, size_t size) noexcept
{
  return rust::Slice(reinterpret_cast<const uint8_t*>(data), size);
}

template<typename C>
inline rust::Slice<const uint8_t>
slice(C c) noexcept
  requires(std::is_trivially_copyable_v<C>)
{
  return rust::Slice(reinterpret_cast<const uint8_t*>(c.data()),
                     static_cast<size_t>(c.size()));
}

template<typename C>
inline rust::Slice<const uint8_t>
slice(const C& c) noexcept
  requires(!std::is_trivially_copyable_v<C>)
{
  return rust::Slice(reinterpret_cast<const uint8_t*>(c.data()),
                     static_cast<size_t>(c.size()));
}

template<typename C>
inline QString
latin1(C c) noexcept
  requires(std::is_trivially_copyable_v<C>)
{
  return QString::fromLatin1(c.data(), static_cast<qsizetype>(c.size()));
}

template<typename C>
inline QString
latin1(const C& c) noexcept
  requires(!std::is_trivially_copyable_v<C>)
{
  return QString::fromLatin1(c.data(), static_cast<qsizetype>(c.size()));
}

template<typename C>
inline QString
utf8(C c) noexcept
  requires(std::is_trivially_copyable_v<C>)
{
  return QString::fromUtf8(c.data(), static_cast<qsizetype>(c.size()));
}

template<typename C>
inline QString
utf8(const C& c) noexcept
  requires(!std::is_trivially_copyable_v<C>)
{
  return QString::fromUtf8(c.data(), static_cast<qsizetype>(c.size()));
}

} // namespace bytes
