#pragma once
#include <algorithm>
#include <limits>

template<typename T, typename U>
constexpr T
clamped_cast(U n) noexcept
{
  return static_cast<T>(
    std::clamp<U>(n,
                  static_cast<U>(std::numeric_limits<T>::min()),
                  static_cast<U>(std::numeric_limits<T>::max())));
}

template<typename Source, typename EnumValue>
void (Source::* enum_slot_cast(void (Source::*slot)(EnumValue)))(int)
  requires(std::is_same_v<std::underlying_type_t<EnumValue>, int>)
{
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: the underlying type of EnumValue is int
  return reinterpret_cast<void (Source::*)(int)>(slot);
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}
