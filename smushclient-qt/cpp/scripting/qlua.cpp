#include "qlua.h"
#include <QtCore/QUuid>
#include "miniwindow.h"
extern "C"
{
#include "lauxlib.h"
}

using std::string;
using std::string_view;
using std::vector;

inline QString charString(char c) { return QString::fromUtf8(&c, 1); }
inline QString charString(char16_t c) { return QString::fromUtf16(&c, 1); }
inline QString charString(char32_t c) { return QString::fromUcs4(&c, 1); }

bool checkIsSome(lua_State *L, int idx, int type, const char *name)
{
  const int actualType = lua_type(L, idx);
  if (actualType <= 0)
    return false;
  luaL_argexpected(L, actualType == type, idx, name);
  return true;
}

lua_Integer toInt(lua_State *L, int idx)
{
  int isInt;
  const int result = lua_tointegerx(L, idx, &isInt);
  luaL_argexpected(L, isInt, idx, "integer");
  return result;
}

QColor toColor(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  if (len == 0)
    return QColor();

  const QColor color = QColor::fromString(QAnyStringView(message, len));
  luaL_argcheck(L, color.isValid(), idx, "invalid color");
  return color;
}

QString toQString(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QString::fromUtf8(message, len);
}

QVariantMap toQMap(lua_State *L, int idx)
{
  QVariantMap map;
  lua_pushnil(L); // first key
  while (lua_next(L, idx) != 0)
    map[toQString(L, -2)] = qlua::getQVariant(L, -1);

  return map;
}

QVariantList toQVariants(lua_State *L, int idx, qsizetype size)
{
  QVariantList variants(size);
  for (lua_Integer i = 1; i <= size; ++i)
  {
    int type = lua_rawgeti(L, idx, i);
    variants.append(qlua::getQVariant(L, -1, type));
    lua_pop(L, 1);
  }
  return variants;
}

string_view toString(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return string_view(message, len);
}

QString qlua::getError(lua_State *L)
{
  size_t len;
  const char *message = lua_tolstring(L, -1, &len);
  return QString::fromUtf8(message, len);
}

QByteArrayView qlua::borrowBytes(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QByteArrayView(message, len);
}

bool qlua::getBool(lua_State *L, int idx)
{
  switch (lua_type(L, idx))
  {
  case LUA_TBOOLEAN:
    return lua_toboolean(L, idx);
  case LUA_TNUMBER:
    if (int isInt, value = lua_tointegerx(L, idx, &isInt); isInt)
      switch (value)
      {
      case 0:
        return false;
      case 1:
        return true;
      }
  }
  luaL_typeerror(L, idx, "boolean"); // exits function
  return false;                      // unrechable
}

bool qlua::getBool(lua_State *L, int idx, bool ifNil)
{
  return checkIsSome(L, idx, LUA_TBOOLEAN, "boolean") ? lua_toboolean(L, idx) : ifNil;
}

Qt::CursorShape qlua::getCursor(lua_State *L, int idx)
{
  switch (getInt(L, idx))
  {
  case -1:
    return Qt::CursorShape::BlankCursor;
  case 0:
    return Qt::CursorShape::ArrowCursor;
  case 1:
    return Qt::CursorShape::OpenHandCursor;
  case 2:
    return Qt::CursorShape::IBeamCursor;
  case 3:
    return Qt::CursorShape::CrossCursor;
  case 4:
    return Qt::CursorShape::WaitCursor;
  case 5:
    return Qt::CursorShape::UpArrowCursor;
  case 6:
    return Qt::CursorShape::SizeFDiagCursor;
  case 7:
    return Qt::CursorShape::SizeBDiagCursor;
  case 8:
    return Qt::CursorShape::SizeHorCursor;
  case 9:
    return Qt::CursorShape::SizeVerCursor;
  case 10:
    return Qt::CursorShape::SizeAllCursor;
  case 11:
    return Qt::CursorShape::ForbiddenCursor;
  case 12:
    return Qt::WhatsThisCursor;
  default:
    luaL_typeerror(L, idx, "cursor code"); // exits function
    return Qt::CursorShape::ArrowCursor;   // unreachable
  }
}

lua_Integer qlua::getInt(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "integer");
  return toInt(L, idx);
}

lua_Integer qlua::getInt(lua_State *L, int idx, lua_Integer ifNil)
{
  return (checkIsSome(L, idx, LUA_TNUMBER, "integer")) ? toInt(L, idx) : ifNil;
}

lua_Number qlua::getNumber(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "number");
  return lua_tonumber(L, idx);
}

lua_Number qlua::getNumber(lua_State *L, int idx, lua_Number ifNil)
{
  return checkIsSome(L, idx, LUA_TNUMBER, "number") ? lua_tonumber(L, idx) : ifNil;
}

QColor qlua::getQColor(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toColor(L, idx);
}

QColor qlua::getQColor(lua_State *L, int idx, QColor ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toColor(L, idx) : ifNil;
}

QString qlua::getQString(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toQString(L, idx);
}

QString qlua::getQString(lua_State *L, int idx, QString ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toQString(L, idx) : ifNil;
}

QVariant qlua::getQVariant(lua_State *L, int idx, int type)
{
  switch (type)
  {
  case LUA_TNONE:
    return QVariant();
  case LUA_TNIL:
    return QVariant(QMetaType::fromType<std::nullptr_t>());
  case LUA_TNUMBER:
    if (int isInt, result = lua_tointegerx(L, idx, &isInt); isInt)
      return QVariant(result);
    return QVariant(lua_tonumber(L, idx));
  case LUA_TBOOLEAN:
    return QVariant(lua_toboolean(L, idx));
  case LUA_TSTRING:
    return QVariant(getQString(L, idx));
  case LUA_TTABLE:
    if (lua_Integer len = lua_rawlen(L, idx))
      return QVariant(toQVariants(L, idx, len));
    if (const QVariantMap map = toQMap(L, idx); map.size())
      return QVariant(map);
    return QVariant(QVariantList(0));
  default:
    return QVariant();
  }
}

string_view qlua::getString(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toString(L, idx);
}

string_view qlua::getString(lua_State *L, int idx, string_view ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toString(L, idx) : ifNil;
}

template <>
MiniWindow::Position qlua::getEnum(lua_State *L, int idx)
{
  const lua_Integer val = getInt(L, idx);
  luaL_argexpected(L, val >= 0 && val <= (lua_Integer)MiniWindow::Position::Tile, idx, "window position enum");
  return (MiniWindow::Position)val;
}

int qlua::loadQString(lua_State *L, const QString &chunk)
{
  const QByteArray utf8 = chunk.toUtf8();
  const char *data = utf8.constData();
  return luaL_loadbuffer(L, data, utf8.size(), data);
}

const char *qlua::pushBytes(lua_State *L, const QByteArray &bytes)
{
  return lua_pushlstring(L, bytes.constData(), bytes.size());
}

void qlua::pushQColor(lua_State *L, const QColor &color)
{
  pushQString(L, color.name());
}

void qlua::pushQHash(lua_State *L, const QVariantHash &variants)
{
  lua_createtable(L, 0, variants.size());
  for (auto it = variants.cbegin(), end = variants.cend(); it != end; ++it)
  {
    pushQString(L, it.key());
    pushQVariant(L, it.value());
    lua_rawset(L, -3);
  }
}

void qlua::pushQMap(lua_State *L, const QVariantMap &variants)
{
  lua_createtable(L, 0, variants.size());
  for (auto it = variants.cbegin(), end = variants.cend(); it != end; ++it)
  {
    pushQString(L, it.key());
    pushQVariant(L, it.value());
    lua_rawset(L, -3);
  }
}

const char *qlua::pushQString(lua_State *L, const QString &string)
{
  return pushBytes(L, string.toUtf8());
}

void qlua::pushQStrings(lua_State *L, const QStringList &strings)
{
  lua_createtable(L, strings.size(), 0);
  lua_Integer i = 1;
  for (const QString &string : strings)
  {
    pushQString(L, string);
    lua_rawseti(L, -2, i);
    ++i;
  }
}

void qlua::pushQVariant(lua_State *L, const QVariant &variant)
{
  QMetaType type = variant.metaType();
  switch (type.id())
  {
  case QMetaType::UnknownType:
  case QMetaType::Nullptr:
    lua_pushnil(L);
    return;
  case QMetaType::Bool:
    lua_pushboolean(L, variant.toBool());
    return;
  case QMetaType::Int:
  case QMetaType::UInt:
  case QMetaType::Long:
  case QMetaType::LongLong:
  case QMetaType::Short:
  case QMetaType::ULong:
  case QMetaType::ULongLong:
  case QMetaType::UShort:
    lua_pushinteger(L, variant.value<lua_Integer>());
    return;
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    lua_pushnumber(L, variant.value<lua_Number>());
    return;
  case QMetaType::QChar:
    pushQString(L, variant.toChar());
    return;
  case QMetaType::QString:
  case QMetaType::QDate:
  case QMetaType::QDateTime:
    pushQString(L, variant.toString());
    return;
  case QMetaType::QByteArray:
    pushBytes(L, variant.toByteArray());
    return;
  case QMetaType::Char:
    pushQString(L, charString(variant.value<char>()));
    return;
  case QMetaType::Char16:
    pushQString(L, charString(variant.value<char16_t>()));
    return;
  case QMetaType::Char32:
    pushQString(L, charString(variant.value<char32_t>()));
    return;
  case QMetaType::SChar:
    pushQString(L, charString((char)variant.value<signed char>()));
    return;
  case QMetaType::UChar:
    pushQString(L, charString((char)variant.value<unsigned char>()));
    return;
  case QMetaType::QColor:
    pushQColor(L, variant.value<QColor>());
    return;
  case QMetaType::QUuid:
    pushQString(L, variant.toUuid().toString());
    return;
  case QMetaType::QStringList:
    pushQStrings(L, variant.toStringList());
    return;
  case QMetaType::QVariantHash:
    pushQHash(L, variant.toHash());
    return;
  case QMetaType::QVariantMap:
    pushQMap(L, variant.toMap());
    return;
  case QMetaType::QVariantList:
    if (variant.canConvert<QStringList>())
      pushQStrings(L, variant.toStringList());
    else
      pushQVariants(L, variant.toList());
    return;
  default:
    if (type.flags().testFlag(QMetaType::IsEnumeration))
      lua_pushinteger(L, variant.toInt());
    else
      lua_pushnil(L);
    return;
  }
}

void qlua::pushQVariants(lua_State *L, const QVariantList &variants)
{
  lua_createtable(L, variants.size(), 0);
  lua_Integer i = 1;
  for (const QVariant &variant : variants)
  {
    pushQVariant(L, variant);
    lua_rawseti(L, -2, i);
    ++i;
  }
}

const char *qlua::pushString(lua_State *L, string_view string)
{
  return lua_pushlstring(L, string.data(), string.size());
}

void qlua::pushStrings(lua_State *L, const vector<string> &strings)
{
  lua_createtable(L, strings.size(), 0);
  int i = 1;
  for (const string &string : strings)
  {
    qlua::pushString(L, string);
    lua_rawseti(L, -2, i);
    ++i;
  }
  return;
}

bool qlua::copyValue(lua_State *fromL, lua_State *toL, int idx)
{
  switch (lua_type(fromL, idx))
  {
  case LUA_TNONE:
    return true;
  case LUA_TNIL:
    lua_pushnil(toL);
    return true;
  case LUA_TBOOLEAN:
    lua_pushboolean(toL, lua_toboolean(fromL, idx));
    return true;
  case LUA_TNUMBER:
    if (int isInt, result = lua_tointegerx(fromL, idx, &isInt); isInt)
      lua_pushinteger(toL, result);
    else
      lua_pushnumber(toL, lua_tonumber(fromL, idx));
    return true;
  case LUA_TSTRING:
  {
    size_t len;
    const char *s = lua_tolstring(fromL, idx, &len);
    lua_pushlstring(toL, s, len);
  }
    return true;
  default:
    return false;
  }
}
