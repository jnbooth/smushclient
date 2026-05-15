#pragma once
#include <iterator>
#include <type_traits>

// Internal helpers for determining whether a value can be pushed to Lua.
namespace qlua::typeHelpers {
namespace entry {
template<typename K, typename V>
struct Traits
{
  using key_type = K;
  using mapped_type = V;
};

template<typename T>
constexpr auto
getTraits(const T& entry) noexcept
{
  const auto& [key, value] = entry;
  return Traits<std::remove_cv_t<decltype(key)>,
                std::remove_cv_t<decltype(value)>>{};
}

} // namespace entry

template<typename T>
using element_t =
  std::remove_cv_t<decltype(*std::data(std::declval<const T&>()))>;

template<typename T>
using entry_traits = decltype(entry::getTraits(std::declval<const T&>()));

template<typename T, typename Enable = void>
struct is_compatible_string : std::false_type
{};

template<typename T>
struct is_compatible_string<
  T,
  std::enable_if_t<std::is_same_v<
    char,
    std::remove_cvref_t<decltype(*std::data(std::declval<const T&>()))>>>>
  : std::true_type
{};

template<typename T>
constexpr bool is_compatible_string_v = is_compatible_string<T>::value;
} // namespace qlua::typeHelpers
