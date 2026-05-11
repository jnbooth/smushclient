#pragma once
#include <limits>

template<typename From, typename To>
constexpr const bool casting_overflows_v =
  std::is_integral_v<To> &&
  (std::is_floating_point_v<From> ||
   (std::is_integral_v<From> &&
    std::numeric_limits<From>::digits > std::numeric_limits<To>::digits));

template<typename From, typename To>
struct casting_overflows : std::bool_constant<casting_overflows_v<To, From>>
{};

template<typename To, typename From>
constexpr To
clamped_cast(From n) noexcept
  requires(casting_overflows_v<From, To>)
{
  constexpr const To toMin =
    std::is_signed_v<From> ? std::numeric_limits<To>::min() : 0;
  constexpr const To toMax = std::numeric_limits<To>::max();
  constexpr const From fromMin = static_cast<From>(toMin);
  constexpr const From fromMax = static_cast<From>(toMax);

  return (n < fromMin) ? toMin : (n > fromMax) ? toMax : static_cast<To>(n);
}

template<typename To, typename From>
constexpr To
clamped_cast(From n) noexcept
  requires(!casting_overflows_v<From, To> && std::is_signed_v<From> &&
           std::is_unsigned_v<To>)
{
  return (n < 0) ? 0 : static_cast<To>(n);
}
