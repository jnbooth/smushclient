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

  using const_iterator = super::const_iterator;
  using iterator = super::iterator;

  using super::at;
  T& at(std::string_view key)
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      throw std::out_of_range("unordered_map::at: key not found");
    }
    return i->second;
  }

  const T& at(std::string_view key) const
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      throw std::out_of_range("unordered_map::at: key not found");
    }
    return i->second;
  }

  using super::erase;
  super::size_type erase(std::string_view key)
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      return 0;
    }
    super::erase(i);
    return 1;
  }

  using super::extract;
  super::node_type extract(std::string_view key)
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      return {};
    }
    return super::extract(i);
  }

  using super::insert_or_assign;
  template<typename M>
  std::pair<iterator, bool> insert_or_assign(std::string_view key, M&& value)
  {
    auto v = try_emplace(key, std::forward<M>(value));
    if (!v.second) {
      v.first->second = std::forward<M>(value);
    }
    return v;
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

  using super::try_emplace;
  template<typename... Args>
  std::pair<iterator, bool> try_emplace(std::string_view key, Args&&... args)
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      return super::emplace(key, std::forward<Args>(args)...);
    }
    return { i, false };
  }

  using super::operator[];
  T& operator[](std::string_view key)
  {
    iterator i = super::find(key);
    if (i == super::end()) {
      return super::operator[](std::string(key));
    }
    return i->second;
  }
};
