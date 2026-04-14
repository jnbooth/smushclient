#pragma once
#include <limits>

template<typename To, typename From>
constexpr To
clamped_cast(From n) noexcept
  requires(std::numeric_limits<To>::is_integer)
{
  using from = std::numeric_limits<From>;
  using to = std::numeric_limits<To>;

  constexpr const bool bounded =
    !from::is_integer || from::digits >= to::digits;

  constexpr const From min = !from::is_signed || !to::is_signed ? 0
                             : bounded ? static_cast<From>(to::min())
                                       : from::min();

  constexpr const From max =
    bounded ? static_cast<From>(to::max()) : from::max();

  return (n < min) ? to::min() : (n > max) ? to::max() : static_cast<To>(n);
}

template<typename T>
concept IntEnum = std::is_same_v<std::underlying_type_t<T>, int>;

template<typename Source, IntEnum EnumValue>
void (Source::* enum_slot_cast(void (Source::*slot)(EnumValue)))(int)
{
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: the underlying type of EnumValue is int
  return reinterpret_cast<void (Source::*)(int)>(slot);
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}
