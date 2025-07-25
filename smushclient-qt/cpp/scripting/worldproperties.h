#pragma once
#include "../lookup.h"
#include <QtCore/QMetaType>
#include <string>
#include <vector>

class WorldProperties {
public:
  static const char *canonicalName(std::string_view key);
  inline static const std::vector<std::string> &numericOptions() {
    return getInstance().numericProps;
  }
  inline static const std::vector<std::string> &stringOptions() {
    return getInstance().stringProps;
  }

private:
  static const WorldProperties &getInstance() {
    static const WorldProperties instance;
    return instance;
  }

  WorldProperties();
  void addProp(const std::string &prop, const QMetaType &type);

private:
  std::vector<std::string> numericProps{};
  std::vector<std::string> stringProps{};
  string_map<std::string> names{};
};
