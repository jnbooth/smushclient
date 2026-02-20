#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct string_hash
{
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  size_t operator()(const char* str) const { return hash_type{}(str); }
  size_t operator()(std::string_view str) const { return hash_type{}(str); }
  size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

template<typename T>
class string_map
  : public std::unordered_map<std::string, T, string_hash, std::equal_to<>>
{
private:
  using super =
    std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

public:
  template<class... Args>
  explicit string_map(Args&&... args)
    : super(std::forward<Args>(args)...)
  {
  }

  using super::erase;

  bool erase(std::string_view key)
  {
    auto search = super::find(key);
    if (search == super::end()) {
      return false;
    }
    super::erase(search);
    return true;
  }

  std::vector<std::string_view> keys() const
  {
    std::vector<std::string_view> list;
    list.reserve(super::size());
    for (const auto& pair : *this) {
      list.emplace_back(pair.first);
    }
    return list;
  }
};
