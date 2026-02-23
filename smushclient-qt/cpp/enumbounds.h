#pragma once

template<typename T>
struct enum_bounds;

#define DECLARE_ENUM_BOUNDS(T, minVal, maxVal)                                 \
  template<>                                                                   \
  struct enum_bounds<T>                                                        \
  {                                                                            \
    static constexpr T min = T::minVal;                                        \
    static constexpr T max = T::maxVal;                                        \
                                                                               \
    static constexpr bool validate(long long value) noexcept                   \
    {                                                                          \
      return value >= static_cast<long long>(min) &&                           \
             value <= static_cast<long long>(max);                             \
    }                                                                          \
  };
