#pragma once

template<typename T>
struct enum_bounds;

#define DECLARE_ENUM_BOUNDS(T, minVal, maxVal)                                 \
  template<>                                                                   \
  struct enum_bounds<T>                                                        \
  {                                                                            \
    static constexpr T min = T::minVal;                                        \
    static constexpr T max = T::maxVal;                                        \
  };
