#pragma once
#include <limits>

template<typename To, typename From>
constexpr To
clamped_cast(From n) noexcept
  requires(std::numeric_limits<From>::is_integer,
           std::numeric_limits<To>::is_integer)
{
  using from_limits = std::numeric_limits<From>;
  using to_limits = std::numeric_limits<To>;

  constexpr const From min = !from_limits::is_signed || !to_limits::is_signed
                               ? 0
                             : from_limits::digits >= to_limits::digits
                               ? static_cast<From>(to_limits::min())
                               : from_limits::min();

  constexpr const From max = from_limits::digits >= to_limits::digits
                               ? static_cast<From>(to_limits::max())
                               : from_limits::max();

  return (n < min)   ? to_limits::min()
         : (n > max) ? to_limits::max()
                     : static_cast<To>(n);
}

constexpr const size_t test = 0;

template<typename Source, typename EnumValue>
void (Source::* enum_slot_cast(void (Source::*slot)(EnumValue)))(int)
  requires(std::is_same_v<std::underlying_type_t<EnumValue>, int>)
{
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: the underlying type of EnumValue is int
  return reinterpret_cast<void (Source::*)(int)>(slot);
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}
