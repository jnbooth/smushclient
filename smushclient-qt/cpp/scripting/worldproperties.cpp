#include "worldproperties.h"
#include <QtCore/QMetaProperty>
#include "cxx-qt-gen/ffi.cxxqt.h"

using std::sort;
using std::string;
using std::unordered_map;
using std::vector;

// Private utils

string toSnakeCase(const string &key)
{
  size_t size = key.size();
  for (char c : key)
    if (isupper(c) || isnumber(c))
      ++size;
  std::string result;
  result.reserve(size);
  bool isFirst = false;
  for (char c : key)
  {
    if (!isupper(c) && !isnumber(c))
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

// Private methods

WorldProperties::WorldProperties()
{
  const QMetaObject &metaObject = World::staticMetaObject;
  int offset = metaObject.propertyOffset();
  int count = metaObject.propertyCount();
  size_t size = count - offset;
  names.reserve(size);
  numericProps.reserve(size);
  stringProps.reserve(size);
  for (int i = offset; i < count; ++i)
  {
    const QMetaProperty prop = metaObject.property(i);
    const string name = prop.name();
    if (!name.rfind("ansi", 0))
      continue;
    const string key = toSnakeCase(name);
    addProp(key, prop.metaType());
    names[key] = name;
  }
  numericProps.shrink_to_fit();
  stringProps.shrink_to_fit();
  sort(numericProps.begin(), numericProps.end());
  sort(stringProps.begin(), stringProps.end());
}

void WorldProperties::addProp(const string &prop, const QMetaType &type)
{
  int id = type.id();
  switch (id)
  {
  case QMetaType::Bool:
  case QMetaType::Int:
  case QMetaType::UInt:
  case QMetaType::Long:
  case QMetaType::LongLong:
  case QMetaType::Short:
  case QMetaType::ULong:
  case QMetaType::ULongLong:
  case QMetaType::UShort:
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    numericProps.push_back(prop);
    break;
  case QMetaType::QChar:
  case QMetaType::QString:
  case QMetaType::QByteArray:
  case QMetaType::Char:
  case QMetaType::Char16:
  case QMetaType::Char32:
  case QMetaType::SChar:
  case QMetaType::UChar:
  case QMetaType::QUuid:
  case QMetaType::QStringList:
    stringProps.push_back(prop);
    break;
  case QMetaType::QColor:
    numericProps.push_back(prop);
    stringProps.push_back(prop);
    break;
  default:
    if (type.flags().testFlag(QMetaType::IsEnumeration))
      numericProps.push_back(prop);
  }
}

unordered_map<string, string> createNameMap()
{
  unordered_map<string, string> map;
  const QMetaObject &metaObject = World::staticMetaObject;
  int offset = metaObject.propertyOffset();
  int count = metaObject.propertyCount();
  size_t size = count - offset;
  map.reserve(size);
  for (int i = offset; i < count; ++i)
  {
    const QMetaProperty prop = metaObject.property(i);
    const string name = prop.name();
    if (!name.rfind("ansi", 0))
      continue;
    const string key = toSnakeCase(name);
    map[key] = name;
  }
  return map;
}

const char *WorldProperties::canonicalName(std::string_view name)
{
  const unordered_map<string, string> &names = getInstance().names;
  auto search = names.find((string)name);
  return (search == names.end()) ? nullptr : search->second.data();
}
