#pragma once

#include <optional>

template<typename T>
struct enum_bounds;

#define DECLARE_ENUM_BOUNDS(T, minVal, maxVal)                                 \
  template<>                                                                   \
  struct enum_bounds<T>                                                        \
  {                                                                            \
    static constexpr const T min = T::minVal;                                  \
    static constexpr const T max = T::maxVal;                                  \
                                                                               \
    static constexpr bool validate(long long value) noexcept                   \
    {                                                                          \
      return value >= static_cast<long long>(min) &&                           \
             value <= static_cast<long long>(max);                             \
    }                                                                          \
  };

template<typename T>
constexpr std::optional<T>
enum_cast(int64_t value) noexcept
{
  if (enum_bounds<T>::validate(value)) [[likely]] {
    return std::optional(static_cast<T>(value));
  }
  return std::nullopt;
}
