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
