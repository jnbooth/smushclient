#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <QtCore/QMetaType>
extern "C"
{
#include "lua.h"
}

class WorldProperties
{
public:
  static const char *canonicalName(std::string_view key);
  inline static const std::vector<std::string> &numericOptions() { return getInstance().numericProps; };
  inline static const std::vector<std::string> &stringOptions() { return getInstance().stringProps; };

private:
  static const WorldProperties &getInstance()
  {
    const static WorldProperties instance;
    return instance;
  }

  std::vector<std::string> numericProps;
  std::vector<std::string> stringProps;
  std::unordered_map<std::string, std::string> names;

  WorldProperties();
  void addProp(const std::string &prop, const QMetaType &type);
};