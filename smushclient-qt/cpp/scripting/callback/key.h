#pragma once
#include "rust/cxx.h"

class PluginCallbackKey
{
public:
  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr PluginCallbackKey(std::string_view routine) noexcept
    : name(routine)
  {
    const size_t n = routine.find('.');
    if (n == std::string_view::npos) {
      return;
    }

    name = routine.substr(0, n);
    property = routine.substr(n + 1);
  }

  constexpr PluginCallbackKey(const std::string& routine) noexcept
    : PluginCallbackKey(std::string_view(routine))
  {
  }

  PluginCallbackKey(rust::Str routine) noexcept
    : PluginCallbackKey(std::string_view(routine.data(), routine.length()))
  {
  }

  constexpr PluginCallbackKey(const rust::String& routine) noexcept
    : PluginCallbackKey(std::string_view(routine.data(), routine.length()))
  {
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

public:
  std::string_view name;
  std::string_view property;
};
