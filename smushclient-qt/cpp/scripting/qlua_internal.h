#pragma once
#include <type_traits>

// Internal helpers for determining whether a value can be pushed to Lua.
namespace qlua::typeHelpers {
namespace chars {
template<typename T>
struct is_char_pointer : std::false_type
{};
template<>
struct is_char_pointer<const char*> : std::true_type
{};
template<>
struct is_char_pointer<char*> : std::true_type
{};

template<typename T>
constexpr bool is_char_pointer_v = is_char_pointer<T>::value;
} // namespace chars

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

template<typename T>
using entry_traits = decltype(getTraits(std::declval<T>()));
} // namespace entry

template<typename T>
using element_t = decltype(*std::declval<T>().begin());

template<typename T, typename Enable = void>
struct is_compatible_string : std::false_type
{};

template<typename T>
struct is_compatible_string<T,
                            std::enable_if_t<chars::is_char_pointer_v<
                              decltype(std::declval<const T&>().data())>>>
  : std::true_type
{};

template<typename T>
constexpr bool is_compatible_string_v = is_compatible_string<T>::value;

template<typename T>
using entry_key_t = typename entry::entry_traits<T>::key_type;

template<typename T>
using entry_value_t = typename entry::entry_traits<T>::mapped_type;
} // namespace qlua::typeHelpers
