#include <string>
#include <vector>
#include <unordered_map>
extern "C"
{
#include "lua.h"
}

class WorldProperties
{
public:
  static const char *canonicalName(const std::string &key);
  inline static const std::vector<std::string> &keys() { return instance.sortedKeys; };

private:
  WorldProperties();
  const static WorldProperties instance;
  std::vector<std::string> sortedKeys;
  std::unordered_map<std::string, std::string> lookup;
};
