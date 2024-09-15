#include <unordered_map>
#include <QtCore/QMetaProperty>
#include "cxx-qt-gen/ffi.cxxqt.h"
#include "worldproperties.h"
#include "qlua.h"

extern "C"
{
#include "lua.h"
}

using std::sort;
using std::string;
using std::vector;

string toSnakeCase(const string &key)
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

const WorldProperties WorldProperties::instance;

WorldProperties::WorldProperties()
{
  const QMetaObject &metaObject = World::staticMetaObject;
  int offset = metaObject.propertyOffset();
  int count = metaObject.propertyCount();
  size_t size = count - offset;
  lookup.reserve(size);
  sortedKeys.reserve(size);
  for (int i = offset; i < count; ++i)
  {
    string name = metaObject.property(i).name();
    if (!name.rfind("ansiColors", 0))
      continue;
    string key = toSnakeCase(name);
    lookup[key] = name;
    sortedKeys.push_back(key);
  }
  sort(sortedKeys.begin(), sortedKeys.end());
}

const char *WorldProperties::canonicalName(const std::string &name)
{
  auto search = instance.lookup.find(name);
  return (search == instance.lookup.end()) ? nullptr : search->second.data();
}
