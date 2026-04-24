#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct string_hash : std::hash<std::string_view>
{
  using is_transparent = void;
};

template<typename T>
class string_map
  : public std::unordered_map<std::string, T, string_hash, std::equal_to<>>
{
private:
  using super =
    std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

public:
  using super::super;

  using super::erase;

  super::size_type erase(std::string_view key)
  {
    auto search = super::find(key);
    if (search == super::end()) {
      return 0;
    }
    super::erase(search);
    return 1;
  }

  std::vector<std::string_view> keys() const
  {
    std::vector<std::string_view> list;
    list.reserve(super::size());
    for (const auto& [key, _] : *this) {
      list.emplace_back(key);
    }
    return list;
  }
};
