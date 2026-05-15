#pragma once
#include <limits>

template<typename From, typename To>
struct casting_overflows
  : std::conjunction<
      // Overflow can only occur when casting TO an integral type.
      std::is_integral<To>,
      std::disjunction<
        // Overflow can occur if casting FROM a floating-point type...
        std::is_floating_point<From>,
        // or FROM an integral type with more digits.
        std::conjunction<
          std::is_integral<From>,
          std::bool_constant<(std::numeric_limits<From>::digits >
                              std::numeric_limits<To>::digits)>>>>
{};

template<typename From, typename To>
constexpr bool casting_overflows_v = casting_overflows<From, To>::value;

template<typename To, typename From>
constexpr To
clamped_cast(From n) noexcept
  requires(casting_overflows_v<From, To>)
{
  constexpr To toMin =
    std::is_signed_v<From> ? std::numeric_limits<To>::min() : 0;
  constexpr To toMax = std::numeric_limits<To>::max();
  constexpr From fromMin = static_cast<From>(toMin);
  constexpr From fromMax = static_cast<From>(toMax);

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

template<typename To, typename From>
[[deprecated("value cannot overflow; use static_cast instead")]]
constexpr To
clamped_cast(From n) noexcept
  requires(!casting_overflows_v<From, To> &&
           (std::is_unsigned_v<From> || std::is_signed_v<To>))
{
  return static_cast<To>(n);
}

template<typename To, typename From>
constexpr const To*
pointer_sign_cast(const From* data) noexcept
  requires(
    std::is_same_v<std::make_unsigned_t<To>, std::make_unsigned_t<From>> &&
    sizeof(From) == sizeof(To))
{
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: `From` and `To` are equivalent types differing only in signedness.
  return reinterpret_cast<const To*>(data);
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

template<typename T>
std::underlying_type_t<T>
to_underlying(T value) noexcept
  requires(std::is_enum_v<T>)
{
  return static_cast<std::underlying_type_t<T>>(value);
}

class QVariant;
template<typename T>
T
qvariant_cast(const QVariant& v);

template<typename T>
T
qvariant_enum_cast(const QVariant& variant)
  requires(std::is_enum_v<T>)
{
  return static_cast<T>(qvariant_cast<std::underlying_type_t<T>>(variant));
}
