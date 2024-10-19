#include "qlua.h"
#include <iostream>
#include <sstream>
#include <QtCore/QUuid>
#include "miniwindow.h"
extern "C"
{
#include "lauxlib.h"
}

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::vector;

// Private utilities

static const QColor customColors[16] = {
    QColor(255, 128, 128),
    QColor(255, 255, 128),
    QColor(128, 255, 128),
    QColor(128, 255, 255),
    QColor(0, 128, 255),
    QColor(255, 128, 192),
    QColor(255, 0, 0),
    QColor(0, 128, 192),
    QColor(255, 0, 255),
    QColor(128, 64, 64),
    QColor(255, 128, 64),
    QColor(0, 128, 128),
    QColor(0, 64, 128),
    QColor(255, 0, 128),
    QColor(0, 128, 0),
    QColor(0, 0, 255),
};

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

int toInt(lua_State *L, int idx)
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
  luaL_argcheck(L, color.isValid(), idx, "valid color");
  return color;
}

QString toQString(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QString::fromUtf8(message, len);
}

QVariantHash toQHash(lua_State *L, int idx)
{
  QVariantHash hash;
  lua_pushnil(L); // first key
  while (lua_next(L, idx) != 0)
  {
    hash[toQString(L, -2)] = qlua::getQVariant(L, -1);
    lua_pop(L, 1);
  }
  return hash;
}

QVariantList toQVariants(lua_State *L, int idx, qsizetype size)
{
  QVariantList variants(size);
  for (int i = 1; i <= size; ++i)
  {
    int type = lua_rawgeti(L, idx, i);
    variants.append(qlua::getQVariant(L, -1, type));
    lua_pop(L, 1);
  }
  return variants;
}

// Public functions

int qlua::expectMaxArgs(lua_State *L, int max)
{
  const int n = lua_gettop(L);
  if (n > max) [[unlikely]]
  {
    qlua::pushQString(L, QStringLiteral("Too many arguments"));
    lua_error(L);
  }
  return n;
}

QString qlua::getError(lua_State *L)
{
  size_t len;
  const char *message = lua_tolstring(L, -1, &len);
  return QString::fromUtf8(message, len);
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
  case LUA_TSTRING:
  {
    const string_view message = toString(L, idx);
    if (message == "n" || message == "N" || message == "0")
      return false;
    if (message == "y" || message == "Y" || message == "1")
      return true;
  }
  }
  luaL_typeerror(L, idx, "boolean"); // exits function
  return false;                      // unreachable
}

bool qlua::getBool(lua_State *L, int idx, bool ifNil)
{
  switch (lua_type(L, idx))
  {
  case LUA_TNONE:
  case LUA_TNIL:
    return ifNil;
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
  case LUA_TSTRING:
  {
    const string_view message = toString(L, idx);
    if (message == "n" || message == "N" || message == "0")
      return false;
    if (message == "y" || message == "Y" || message == "1")
      return true;
  }
  }
  luaL_typeerror(L, idx, "boolean"); // exits function
  return false;                      // unreachable
}

QByteArrayView qlua::getBytes(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QByteArrayView(message, len);
}

int qlua::getInt(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "integer");
  return toInt(L, idx);
}

int qlua::getInt(lua_State *L, int idx, int ifNil)
{
  return (checkIsSome(L, idx, LUA_TNUMBER, "integer")) ? toInt(L, idx) : ifNil;
}

double qlua::getNumber(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "number");
  return lua_tonumber(L, idx);
}

double qlua::getNumber(lua_State *L, int idx, double ifNil)
{
  return checkIsSome(L, idx, LUA_TNUMBER, "number") ? lua_tonumber(L, idx) : ifNil;
}

double qlua::getNumberOrBool(lua_State *L, int idx)
{
  switch (lua_type(L, idx))
  {
  case LUA_TBOOLEAN:
    return lua_toboolean(L, idx);
  case LUA_TNUMBER:
    return lua_tonumber(L, idx);
  }
  luaL_typeerror(L, idx, "number"); // exits function
  return 0;                         // unreachable
}

double qlua::getNumberOrBool(lua_State *L, int idx, double ifNil)
{
  switch (lua_type(L, idx))
  {
  case LUA_TNONE:
  case LUA_TNIL:
    return ifNil;
  case LUA_TBOOLEAN:
    return lua_toboolean(L, idx);
  case LUA_TNUMBER:
    return lua_tonumber(L, idx);
  }
  luaL_typeerror(L, idx, "number"); // exits function
  return 0;                         // unreachable
}

QColor qlua::getQColor(lua_State *L, int idx)
{
  int type = lua_type(L, idx);
  if (type == LUA_TNUMBER)
    return rgbCodeToColor(toInt(L, idx));
  luaL_argexpected(L, type == LUA_TSTRING, idx, "color name or code");
  return toColor(L, idx);
}

QColor qlua::getCustomColor(lua_State *L, int idx)
{
  const int colorIndex = getInt(L, idx);
  return (colorIndex < 0 || colorIndex >= 16) ? QColor() : customColors[colorIndex];
}

QColor qlua::getQColor(lua_State *L, int idx, const QColor &ifNil)
{
  int type = lua_type(L, idx);
  if (type <= 0)
    return ifNil;
  if (type == LUA_TNUMBER)
    return rgbCodeToColor(toInt(L, idx));
  luaL_argexpected(L, type == LUA_TSTRING, idx, "color name or code");
  return toColor(L, idx);
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
    if (int len = lua_rawlen(L, idx))
      return QVariant(toQVariants(L, idx, len));
    if (const QVariantHash hash = toQHash(L, idx); hash.size())
      return QVariant(hash);
    return QVariant(QVariantList(0));
  default:
    return QVariant();
  }
}

optional<QString> qlua::getScriptName(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  if (len == 0)
    return QString();

  const int globalType = lua_getglobal(L, message);
  lua_pop(L, 1);
  if (globalType != LUA_TFUNCTION)
    return nullopt;

  return QString::fromUtf8(message, len);
}

string_view qlua::toString(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return string_view(message, len);
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

QByteArray qlua::concatBytes(lua_State *L)
{
  const int n = lua_gettop(L);
  lua_Unsigned messageSize = 2; // an extra 2 for newline characters
  for (int i = 1; i <= n; ++i)
  {
    luaL_argexpected(L, lua_type(L, i) == LUA_TSTRING, i, "string");
    messageSize += lua_rawlen(L, i);
  }
  QByteArray bytes;
  bytes.reserve(messageSize);
  size_t chunkLen;
  for (int i = 1; i <= n; ++i)
    bytes.append(lua_tolstring(L, i, &chunkLen), chunkLen);

  return bytes;
}

QString qlua::concatStrings(lua_State *L, const QString &delimiter)
{
  const int n = lua_gettop(L);
  bool needsToString = true;
  QString text;
  size_t sLen;
  for (int i = 1; i <= n; ++i)
  {
    if (i > 1)
      text.append(delimiter);
    switch (lua_type(L, i))
    {
    case LUA_TNIL:
      text.append(QStringLiteral("nil"));
      break;
    case LUA_TBOOLEAN:
      text.append(lua_toboolean(L, i) ? QStringLiteral("true") : QStringLiteral("false"));
      break;
    case LUA_TNUMBER:
      if (int isInt, result = lua_tointegerx(L, i, &isInt); isInt)
        text.append(QString::number(result));
      else
        text.append(QString::number(lua_tonumber(L, i)));
      break;
    case LUA_TSTRING:
      text.append(QUtf8StringView(lua_tolstring(L, i, &sLen), sLen));
      break;
    default:
      if (needsToString)
      {
        needsToString = false;
        lua_getglobal(L, "tostring");
      }
      lua_pushvalue(L, -1); // tostring
      lua_pushvalue(L, i);  // argument
      lua_call(L, 1, 1);
      if (const char *s = lua_tolstring(L, -1, &sLen); s)
      {
        text.append(QUtf8StringView(s, sLen));
        lua_pop(L, 1);
        break;
      }
      lua_pushstring(L, "'tostring' must return a string to be concatenated");
      lua_error(L);
    }
  }
  if (!needsToString)
    lua_pop(L, 1);
  return text;
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
  lua_pushinteger(L, colorToRgbCode(color));
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
  int i = 1;
  for (const QString &string : strings)
  {
    pushQString(L, string);
    lua_rawseti(L, -2, i);
    ++i;
  }
}

void qlua::pushQVariant(lua_State *L, const QVariant &variant, bool intBools)
{
  QMetaType type = variant.metaType();
  switch (type.id())
  {
  case QMetaType::UnknownType:
  case QMetaType::Nullptr:
    lua_pushnil(L);
    return;
  case QMetaType::Bool:
    if (intBools)
      lua_pushinteger(L, variant.toBool());
    else
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
    lua_pushinteger(L, variant.toInt());
    return;
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    lua_pushnumber(L, variant.toDouble());
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
  int i = 1;
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

QLine qlua::getQLine(lua_State *L, int idxX1, int idxY1, int idxX2, int idxY2)
{
  return QLine(getInt(L, idxX1), getInt(L, idxY1), getInt(L, idxX2), getInt(L, idxY2));
}

QLineF qlua::getQLineF(lua_State *L, int idxX1, int idxY1, int idxX2, int idxY2)
{
  return QLineF(getNumber(L, idxX1), getNumber(L, idxY1), getNumber(L, idxX2), getNumber(L, idxY2));
}

QMargins qlua::getQMargins(lua_State *L, int idxLeft, int idxTop, int idxRight, int idxBottom)
{
  return QMargins(getInt(L, idxLeft), getInt(L, idxTop), getInt(L, idxRight), getInt(L, idxBottom));
}

QMarginsF qlua::getQMarginsF(lua_State *L, int idxLeft, int idxTop, int idxRight, int idxBottom)
{
  return QMarginsF(getNumber(L, idxLeft), getNumber(L, idxTop), getNumber(L, idxRight), getNumber(L, idxBottom));
}

QPoint qlua::getQPoint(lua_State *L, int idxX, int idxY)
{
  return QPoint(getInt(L, idxX), getInt(L, idxY));
}

QPointF qlua::getQPointF(lua_State *L, int idxX, int idxY)
{
  return QPointF(getNumber(L, idxX), getNumber(L, idxY));
}

QSize qlua::getQSize(lua_State *L, int idxWidth, int idxHeight)
{
  return QSize(getInt(L, idxWidth), getInt(L, idxHeight));
}

QSizeF qlua::getQSizeF(lua_State *L, int idxWidth, int idxHeight)
{
  return QSizeF(getNumber(L, idxWidth), getNumber(L, idxHeight));
}

QRect qlua::getQRect(lua_State *L, int idxLeft, int idxTop, int idxWidth, int idxHeight)
{
  return QRect(getInt(L, idxLeft), getInt(L, idxTop), getInt(L, idxWidth), getInt(L, idxHeight));
}

QRectF qlua::getQRectF(lua_State *L, int idxLeft, int idxTop, int idxWidth, int idxHeight)
{
  return QRectF(getNumber(L, idxLeft), getNumber(L, idxTop), getNumber(L, idxWidth), getNumber(L, idxHeight));
}

template <typename T, T MIN, T MAX>
inline optional<T> getEnum(lua_State *L, int idx, optional<T> ifNil = nullopt)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer"))
    return ifNil;
  const int val = toInt(L, idx);
  if (val < (int)MIN || val > (int)MAX) [[unlikely]]
    return nullopt;
  return (T)val;
}

optional<Qt::BrushStyle> qlua::getBrush(lua_State *L, int idx, optional<Qt::BrushStyle> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer"))
    return ifNil;

  switch (toInt(L, idx))
  {
  case (int)ScriptBrush::SolidPattern:
    return Qt::BrushStyle::SolidPattern;
  case (int)ScriptBrush::NoBrush:
    return Qt::BrushStyle::NoBrush;
  case (int)ScriptBrush::HorPattern:
    return Qt::BrushStyle::HorPattern;
  case (int)ScriptBrush::VerPattern:
    return Qt::BrushStyle::VerPattern;
  case (int)ScriptBrush::FDiagPattern:
    return Qt::BrushStyle::FDiagPattern;
  case (int)ScriptBrush::BDiagPattern:
    return Qt::BrushStyle::BDiagPattern;
  case (int)ScriptBrush::CrossPattern:
    return Qt::BrushStyle::CrossPattern;
  case (int)ScriptBrush::DiagCrossPattern:
    return Qt::BrushStyle::DiagCrossPattern;
  case (int)ScriptBrush::Dense4Pattern:
    return Qt::BrushStyle::Dense4Pattern;
  case (int)ScriptBrush::Dense2Pattern:
    return Qt::BrushStyle::Dense2Pattern;
  case (int)ScriptBrush::Dense1Pattern:
    return Qt::BrushStyle::Dense1Pattern;
  case (int)ScriptBrush::HorWaves:
    return Qt::BrushStyle::HorPattern;
  case (int)ScriptBrush::VerWaves:
    return Qt::BrushStyle::VerPattern;
  default:
    return nullopt;
  }
}

optional<Qt::CursorShape> qlua::getCursor(lua_State *L, int idx, optional<Qt::CursorShape> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer"))
    return ifNil;

  switch (toInt(L, idx))
  {
  case (int)ScriptCursor::BlankCursor:
    return Qt::CursorShape::BlankCursor;
  case (int)ScriptCursor::ArrowCursor:
    return Qt::CursorShape::ArrowCursor;
  case (int)ScriptCursor::OpenHandCursor:
    return Qt::CursorShape::OpenHandCursor;
  case (int)ScriptCursor::IBeamCursor:
    return Qt::CursorShape::IBeamCursor;
  case (int)ScriptCursor::CrossCursor:
    return Qt::CursorShape::CrossCursor;
  case (int)ScriptCursor::WaitCursor:
    return Qt::CursorShape::WaitCursor;
  case (int)ScriptCursor::UpArrowCursor:
    return Qt::CursorShape::UpArrowCursor;
  case (int)ScriptCursor::SizeFDiagCursor:
    return Qt::CursorShape::SizeFDiagCursor;
  case (int)ScriptCursor::SizeBDiagCursor:
    return Qt::CursorShape::SizeBDiagCursor;
  case (int)ScriptCursor::SizeHorCursor:
    return Qt::CursorShape::SizeHorCursor;
  case (int)ScriptCursor::SizeVerCursor:
    return Qt::CursorShape::SizeVerCursor;
  case (int)ScriptCursor::SizeAllCursor:
    return Qt::CursorShape::SizeAllCursor;
  case (int)ScriptCursor::ForbiddenCursor:
    return Qt::CursorShape::ForbiddenCursor;
  case (int)ScriptCursor::WhatsThisCursor:
    return Qt::WhatsThisCursor;
  default:
    return nullopt;
  }
}

optional<CircleOp> qlua::getCircleOp(lua_State *L, int idx)
{
  return getEnum<CircleOp, CircleOp::Ellipse, CircleOp::Pie>(L, idx, nullopt);
}

optional<MiniWindow::DrawImageMode> qlua::getDrawImageMode(
    lua_State *L,
    int idx,
    optional<MiniWindow::DrawImageMode> ifNil)
{
  return getEnum<
      MiniWindow::DrawImageMode,
      MiniWindow::DrawImageMode::Copy,
      MiniWindow::DrawImageMode::CopyTransparent>(L, idx, ifNil);
}

optional<QFont::StyleHint> qlua::getFontHint(lua_State *L, int idx, optional<QFont::StyleHint> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer"))
    return ifNil;

  const int style = toInt(L, idx);
  switch (style & 0xF) // pitch
  {
  case FontPitchFlag::Default:
  case FontPitchFlag::Fixed:
  case FontPitchFlag::Variable:
    break;
  case FontPitchFlag::Monospace:
    return QFont::StyleHint::Monospace;
  default:
    return nullopt;
  }

  switch (style & ~0xF)
  {
  case FontFamilyFlag::Roman:
    return QFont::StyleHint::Serif;
  case FontFamilyFlag::Swiss:
    return QFont::StyleHint::SansSerif;
  case FontFamilyFlag::Modern:
    return QFont::StyleHint::TypeWriter;
  case FontFamilyFlag::Script:
    return QFont::StyleHint::Cursive;
  case FontFamilyFlag::Decorative:
    return QFont::StyleHint::Decorative;
  default:
    return nullopt;
  }
}

constexpr Qt::PenStyle getPenStyle(int style) noexcept
{
  switch (style & 0xFF)
  {
  case PenStyleFlag::SolidLine:
    return Qt::PenStyle::SolidLine;
  case PenStyleFlag::DashLine:
    return Qt::PenStyle::DashLine;
  case PenStyleFlag::DotLine:
    return Qt::PenStyle::DotLine;
  case PenStyleFlag::DashDotLine:
    return Qt::PenStyle::DashDotLine;
  case PenStyleFlag::DashDotDotLine:
    return Qt::PenStyle::DashDotDotLine;
  case PenStyleFlag::NoPen:
    return Qt::PenStyle::NoPen;
  case PenStyleFlag::InsideFrame:
    return Qt::PenStyle::SolidLine;
  default:
    return Qt::PenStyle::MPenStyle;
  }
}

constexpr Qt::PenCapStyle getPenCap(int style) noexcept
{
  switch (style & 0xF00)
  {
  case PenCapFlag::RoundCap:
    return Qt::PenCapStyle::RoundCap;
  case PenCapFlag::SquareCap:
    return Qt::PenCapStyle::SquareCap;
  case PenCapFlag::FlatCap:
    return Qt::PenCapStyle::FlatCap;
  default:
    return Qt::PenCapStyle::MPenCapStyle;
  }
}

constexpr Qt::PenJoinStyle getPenJoin(int style) noexcept
{
  switch (style & ~0XFFF)
  {
  case PenJoinFlag::RoundJoin:
    return Qt::PenJoinStyle::RoundJoin;
  case PenJoinFlag::BevelJoin:
    return Qt::PenJoinStyle::BevelJoin;
  case PenJoinFlag::MiterJoin:
    return Qt::PenJoinStyle::MiterJoin;
  default:
    return Qt::PenJoinStyle::MPenJoinStyle;
  }
}

optional<QPen> qlua::getPen(lua_State *L, int idxColor, int idxStyle, int idxWidth)
{
  const QColor color = getQColor(L, idxColor);
  const int style = getInt(L, idxStyle);
  const double width = getNumber(L, idxWidth);
  if (style < 0 || width < 0) [[unlikely]]
    return nullopt;

  if (style == 0)
    return QPen(
        color,
        width,
        Qt::PenStyle::SolidLine,
        Qt::PenCapStyle::RoundCap,
        Qt::PenJoinStyle::RoundJoin);

  const Qt::PenStyle penStyle = getPenStyle(style);
  const Qt::PenCapStyle capStyle = getPenCap(style);
  const Qt::PenJoinStyle joinStyle = getPenJoin(style);
  if (
      penStyle == Qt::PenStyle::MPenStyle ||
      capStyle == Qt::PenCapStyle::MPenCapStyle ||
      joinStyle == Qt::PenJoinStyle::MPenJoinStyle) [[unlikely]]
    return nullopt;

  return QPen(color, width, penStyle, capStyle, joinStyle);
}

optional<QPolygonF> qlua::getQPolygonF(lua_State *L, int idx)
{
  const string_view s = getString(L, idx);
  const qsizetype commaCount = std::count(s.cbegin(), s.cend(), ',');
  if (commaCount % 2 == 0 || commaCount < 3) [[unlikely]]
    return nullopt;
  QList<QPointF> points;
  points.reserve((commaCount + 1) / 2);
  std::istringstream stream((string)s);
  for (string sX, sY; std::getline(stream, sX, ',') && std::getline(stream, sY, ',');)
  {
    qreal dX = stod(sX);
    qreal dY = stod(sY);
    if (!std::isfinite(dX) || !std::isfinite(dY)) [[unlikely]]
      return nullopt;
    points.append(QPointF(dX, dY));
  }
  return QPolygonF(points);
}

optional<RectOp> qlua::getRectOp(lua_State *L, int idx)
{
  return getEnum<RectOp, RectOp::Frame, RectOp::FloodFillSurface>(L, idx);
}

optional<SendTarget> qlua::getSendTarget(lua_State *L, int idx, optional<SendTarget> ifNil)
{
  return getEnum<SendTarget, SendTarget::World, SendTarget::ScriptAfterOmit>(L, idx, ifNil);
}

optional<MiniWindow::Position> qlua::getWindowPosition(
    lua_State *L,
    int idx,
    optional<MiniWindow::Position> ifNil)
{
  return getEnum<
      MiniWindow::Position,
      MiniWindow::Position::OutputStretch,
      MiniWindow::Position::Tile>(L, idx, ifNil);
}
