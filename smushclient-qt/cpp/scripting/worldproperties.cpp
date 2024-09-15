#include <string>
#include <unordered_map>
#include <QtCore/QMetaProperty>
#include "cxx-qt-gen/ffi.cxxqt.h"
#include "worldproperties.h"
#include "qlua.h"

extern "C"
{
#include "lua.h"
}

using std::string;
using std::unordered_map;

std::string toSnakeCase(const std::string &key)
{
  size_t size = key.size();
  for (char c : key)
    if (isupper(c))
      ++size;
  std::string result;
  result.reserve(size);
  bool isFirst = false;
  for (char c : key)
  {
    if (!isupper(c))
    {
      result.push_back(c);
      continue;
    }
    if (isFirst)
      isFirst = false;
    else
      result.push_back('_');
    result.push_back(tolower(c));
  }
  return result;
}

unordered_map<string, string> getProperties()
{
  const QMetaObject &metaObject = World::staticMetaObject;
  int offset = metaObject.propertyOffset();
  int count = metaObject.propertyCount();
  unordered_map<string, string> properties;
  properties.reserve(count - offset);
  for (int i = offset; i < count; ++i)
  {
    string name = metaObject.property(i).name();
    if (name.rfind("ansiColors", 0))
      properties[toSnakeCase(name)] = name;
  }
  return properties;
}

static const unordered_map<string, string> properties = getProperties();

const char *canonicalProperty(const std::string &name)
{
  auto search = properties.find(name);
  return (search == properties.end()) ? nullptr : search->second.data();
}

void pushPropertiesList(lua_State *L)
{
  lua_createtable(L, properties.size(), 0);
  int i = 1;
  for (auto it = properties.cbegin(), end = properties.cend(); it != end; ++it, ++i)
  {
    qlua::pushString(L, it->first);
    lua_rawseti(L, -2, i);
  }
  return;
}
