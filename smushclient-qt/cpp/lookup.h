#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>

struct string_hash
{
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  inline std::size_t operator()(const char *str) const { return hash_type{}(str); }
  inline std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
  inline std::size_t operator()(std::string const &str) const { return hash_type{}(str); }
};

using string_set = std::unordered_set<std::string, string_hash, std::equal_to<>>;

template <typename T>
using string_map = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;
