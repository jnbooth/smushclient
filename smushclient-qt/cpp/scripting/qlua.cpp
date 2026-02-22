#include "qlua.h"
#include "../casting.h"
#include "smushclient_qt/src/ffi/sender.cxxqt.h"
#include <QtCore/QUuid>
#include <cmath>
#include <sstream>
extern "C"
{
#include "lauxlib.h"
}

using std::array;
using std::nullopt;
using std::optional;
using std::string;
using std::string_view;

// Private utilities

constexpr int FALSE = 0;
constexpr int TRUE = 1;

static const array<QColor, 16> customColors{
  QColor(255, 128, 128), QColor(255, 255, 128), QColor(128, 255, 128),
  QColor(128, 255, 255), QColor(0, 128, 255),   QColor(255, 128, 192),
  QColor(255, 0, 0),     QColor(0, 128, 192),   QColor(255, 0, 255),
  QColor(128, 64, 64),   QColor(255, 128, 64),  QColor(0, 128, 128),
  QColor(0, 64, 128),    QColor(255, 0, 128),   QColor(0, 128, 0),
  QColor(0, 0, 255),
};

namespace {
bool
checkIsSome(lua_State* L, int idx, int type, const char* name)
{
  const int actualType = lua_type(L, idx);
  if (actualType <= 0) {
    return false;
  }
  luaL_argexpected(L, actualType == type, idx, name);
  return true;
}

inline bool
isScriptName(lua_State* L, string_view name)
{
  const size_t dotIndex = name.find('.');
  if (dotIndex == string_view::npos) {
    const char* namePtr = name.data();
    const bool isFunction = lua_getglobal(L, namePtr) != LUA_TNIL;
    lua_pop(L, 1);
    return isFunction;
  }
  const string tableName(name.substr(0, dotIndex));
  if (lua_getglobal(L, tableName.c_str()) != LUA_TTABLE) {
    lua_pop(L, 1);
    return false;
  }
  const char* propertyPtr = name.substr(dotIndex + 1).data();
  const bool isFunction = lua_getfield(L, -1, propertyPtr) == LUA_TFUNCTION;
  lua_pop(L, 2);
  return isFunction;
}

bool
toBool(lua_State* L, int idx, int type)
{
  switch (type) {
    case LUA_TBOOLEAN:
      return lua_toboolean(L, idx) == TRUE;
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer value = lua_tointegerx(L, idx, &isInt);
      if (isInt == FALSE) {
        break;
      }
      return value == TRUE;
    }
    case LUA_TSTRING: {
      const string_view message = qlua::toString(L, idx);
      if (message == "n" || message == "N" || message == "0") {
        return false;
      }
      if (message == "y" || message == "Y" || message == "1") {
        return true;
      }
      break;
    }
    default:
      break;
  }
  luaL_typeerror(L, idx, "boolean"); // exits function
  return false;                      // unreachable
}

inline lua_Integer
toInteger(lua_State* L, int idx)
{
  int isInt;
  const lua_Integer result = lua_tointegerx(L, idx, &isInt);
  luaL_argexpected(L, isInt, idx, "integer");
  return result;
}

inline lua_Number
toNumber(lua_State* L, int idx)
{
  const lua_Number result = lua_tonumber(L, idx);
  luaL_argexpected(L, std::isfinite(result), idx, "finite number");
  return result;
}

QColor
toQColor(lua_State* L, int idx, int ltype)
{
  switch (ltype) {
    case LUA_TSTRING: {
      size_t len;
      const char* message = lua_tolstring(L, idx, &len);
      if (len == 0) {
        return QColor();
      }
      const QColor color = QColor::fromString(
        QAnyStringView(message, static_cast<qsizetype>(len)));
      luaL_argcheck(L, color.isValid(), idx, "valid color");
      return color;
    }
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer rgb = lua_tointegerx(L, idx, &isInt);
      if (isInt == FALSE) {
        break;
      }
      if (rgb == -1 || rgb == 0xFFFFFFFF) {
        return QColor();
      }
      luaL_argcheck(L, rgb >= 0 && rgb <= 0xFFFFFF, idx, "valid color");
      const int code = static_cast<int>(rgb);
      return QColor(code & 0xFF, (code >> 8) & 0xFF, (code >> 16) & 0xFF);
    }
    default:
      break;
  }
  luaL_typeerror(L, idx, "string or integer");
  return QColor();
}

inline QString
toQString(lua_State* L, int idx)
{
  size_t len;
  const char* message = lua_tolstring(L, idx, &len);
  return QString::fromUtf8(message, static_cast<qsizetype>(len));
}

QVariant
toQVariant(lua_State* L, int idx, int type)
{
  switch (type) {
    case LUA_TNONE:
      return QVariant();
    case LUA_TNIL:
      return QVariant(QMetaType::fromType<nullptr_t>());
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer intResult = lua_tointegerx(L, idx, &isInt);
      return isInt == TRUE ? QVariant(intResult)
                           : QVariant(lua_tonumber(L, idx));
    }
    case LUA_TBOOLEAN:
      return QVariant(lua_toboolean(L, idx));
    case LUA_TSTRING:
      return QVariant(toQString(L, idx));
    case LUA_TTABLE: {
      if (lua_Unsigned len = lua_rawlen(L, idx)) {
        QVariantList variants(static_cast<qsizetype>(len));
        const lua_Integer max = static_cast<lua_Integer>(len);
        for (lua_Integer i = 1; i <= max; ++i) {
          const int type = lua_rawgeti(L, idx, i);
          variants.append(toQVariant(L, -1, type));
          lua_pop(L, 1);
        }
        return variants;
      }
      QVariantHash hash;
      lua_pushnil(L); // first key
      while (lua_next(L, idx) != 0) {
        hash[toQString(L, -2)] = toQVariant(L, -1, lua_type(L, -1));
        lua_pop(L, 1);
      }
      if (!hash.empty()) {
        return QVariant(hash);
      }
      return QVariant(QVariantList(0));
    }
    default:
      return QVariant();
  }
}

constexpr optional<Qt::PenStyle>
getPenStyle(lua_Integer style) noexcept
{
  switch (static_cast<PenStyle>(style & 0xFF)) {
    case PenStyle::SolidLine:
      return Qt::PenStyle::SolidLine;
    case PenStyle::DashLine:
      return Qt::PenStyle::DashLine;
    case PenStyle::DotLine:
      return Qt::PenStyle::DotLine;
    case PenStyle::DashDotLine:
      return Qt::PenStyle::DashDotLine;
    case PenStyle::DashDotDotLine:
      return Qt::PenStyle::DashDotDotLine;
    case PenStyle::NoPen:
      return Qt::PenStyle::NoPen;
    case PenStyle::InsideFrame:
      return Qt::PenStyle::SolidLine;
    default:
      return nullopt;
  }
}

constexpr optional<Qt::PenCapStyle>
getPenCap(lua_Integer style) noexcept
{
  switch (static_cast<PenCap>(style & 0xF00)) {
    case PenCap::RoundCap:
      return Qt::PenCapStyle::RoundCap;
    case PenCap::SquareCap:
      return Qt::PenCapStyle::SquareCap;
    case PenCap::FlatCap:
      return Qt::PenCapStyle::FlatCap;
    default:
      return nullopt;
  }
}

constexpr optional<Qt::PenJoinStyle>
getPenJoin(lua_Integer style) noexcept
{
  switch (static_cast<PenJoin>(style & ~0XFFF)) {
    case PenJoin::RoundJoin:
      return Qt::PenJoinStyle::RoundJoin;
    case PenJoin::BevelJoin:
      return Qt::PenJoinStyle::BevelJoin;
    case PenJoin::MiterJoin:
      return Qt::PenJoinStyle::MiterJoin;
    default:
      return nullopt;
  }
}
} // namespace

// Public functions

int
qlua::expectMaxArgs(lua_State* L, int max)
{
  const int n = lua_gettop(L);
  if (n > max) [[unlikely]] {
    qlua::push(L, "Too many arguments");
    lua_error(L);
  }
  return n;
}

bool
qlua::getBool(lua_State* L, int idx)
{
  return toBool(L, idx, lua_type(L, idx));
}

bool
qlua::getBool(lua_State* L, int idx, bool ifNil)
{
  const int type = lua_type(L, idx);
  if (type == LUA_TNONE || type == LUA_TNIL) {
    return ifNil;
  }
  return toBool(L, idx, type);
}

QByteArrayView
qlua::getBytes(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char* message = lua_tolstring(L, idx, &len);
  return QByteArrayView(message, static_cast<qsizetype>(len));
}

lua_Integer
qlua::getInteger(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "integer");
  return toInteger(L, idx);
}

lua_Integer
qlua::getInteger(lua_State* L, int idx, lua_Integer ifNil)
{
  return (checkIsSome(L, idx, LUA_TNUMBER, "integer")) ? toInteger(L, idx)
                                                       : ifNil;
}

int
qlua::getInt(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "integer");
  return clamped_cast<int>(toInteger(L, idx));
}

int
qlua::getInt(lua_State* L, int idx, int ifNil)
{
  return (checkIsSome(L, idx, LUA_TNUMBER, "integer"))
           ? clamped_cast<int>(toInteger(L, idx))
           : ifNil;
}

lua_Integer
qlua::getIntegerOrBool(lua_State* L, int idx)
{
  switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN:
      return static_cast<lua_Integer>(lua_toboolean(L, idx));
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer value = lua_tointegerx(L, idx, &isInt);
      if (isInt == FALSE) {
        break;
      }
      return value;
    }
    default:
      break;
  }
  luaL_typeerror(L, idx, "integer"); // exits function
  return 0;                          // unreachable
}

lua_Integer
qlua::getIntegerOrBool(lua_State* L, int idx, lua_Integer ifNil)
{
  switch (lua_type(L, idx)) {
    case LUA_TNONE:
    case LUA_TNIL:
      return ifNil;
    case LUA_TBOOLEAN:
      return static_cast<lua_Integer>(lua_toboolean(L, idx));
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer value = lua_tointegerx(L, idx, &isInt);
      if (isInt == FALSE) {
        break;
      }
      return value;
    }
    default:
      break;
  }
  luaL_typeerror(L, idx, "integer"); // exits function
  return 0;                          // unreachable
}

lua_Number
qlua::getNumber(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "number");
  return toNumber(L, idx);
}

lua_Number
qlua::getNumber(lua_State* L, int idx, lua_Number ifNil)
{
  return checkIsSome(L, idx, LUA_TNUMBER, "number") ? toNumber(L, idx) : ifNil;
}

QColor
qlua::getCustomColor(lua_State* L, int idx)
{
  const lua_Integer colorIndex = getInteger(L, idx);
  return (colorIndex < 0 || colorIndex >= 16) ? QColor()
                                              : customColors.at(colorIndex);
}

QColor
qlua::getQColor(lua_State* L, int idx)
{
  return toQColor(L, idx, lua_type(L, idx));
}

QColor
qlua::getQColor(lua_State* L, int idx, const QColor& ifNil)
{
  const int type = lua_type(L, idx);
  return type >= 0 ? toQColor(L, idx, type) : ifNil;
}

QString
qlua::getQString(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toQString(L, idx);
}

QString
qlua::getQString(lua_State* L, int idx, const QString& ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toQString(L, idx) : ifNil;
}

QVariant
qlua::getQVariant(lua_State* L, int idx)
{
  return toQVariant(L, idx, lua_type(L, idx));
}

optional<string_view>
qlua::getScriptName(lua_State* L, int idx)
{
  const string_view name = qlua::getString(L, idx);

  if (name.empty() || isScriptName(L, name)) {
    return name;
  }

  return nullopt;
}

string_view
qlua::toString(lua_State* L, int idx)
{
  size_t len;
  const char* message = lua_tolstring(L, idx, &len);
  return string_view(message, len);
}

string_view
qlua::getString(lua_State* L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toString(L, idx);
}

string_view
qlua::getString(lua_State* L, int idx, string_view ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toString(L, idx) : ifNil;
}

QByteArray
qlua::concatBytes(lua_State* L)
{
  const int n = lua_gettop(L);
  lua_Unsigned messageSize = 2; // an extra 2 for newline characters
  for (int i = 1; i <= n; ++i) {
    luaL_argexpected(L, lua_type(L, i) == LUA_TSTRING, i, "string");
    messageSize += lua_rawlen(L, i);
  }
  QByteArray bytes;
  bytes.reserve(static_cast<qsizetype>(messageSize));
  size_t chunkLen;
  for (int i = 1; i <= n; ++i) {
    const char* data = lua_tolstring(L, i, &chunkLen);
    bytes.append(data, static_cast<qsizetype>(chunkLen));
  }

  return bytes;
}

string
qlua::concatStrings(lua_State* L)
{
  const int n = lua_gettop(L);
  int isInt;
  bool needsToString = true;
  size_t sLen;
  std::ostringstream out;
  for (int i = 1; i <= n; ++i) {
    switch (lua_type(L, i)) {
      case LUA_TNIL:
        out << "nil";
        break;
      case LUA_TBOOLEAN:
        out << (lua_toboolean(L, i) == TRUE ? "true" : "false");
        break;
      case LUA_TNUMBER: {
        const lua_Integer result = lua_tointegerx(L, i, &isInt);
        if (isInt == TRUE) {
          out << result;
        } else {
          out << lua_tonumber(L, i);
        }
        break;
      }
      case LUA_TSTRING: {
        const char* data = lua_tolstring(L, i, &sLen);
        out << string_view(data, sLen);
        break;
      }
      default:
        if (needsToString) {
          needsToString = false;
          lua_getglobal(L, "tostring");
        }
        lua_pushvalue(L, -1); // tostring
        lua_pushvalue(L, i);  // argument
        lua_call(L, 1, 1);
        if (const char* s = lua_tolstring(L, -1, &sLen); s) {
          out << string_view(s, sLen);
          lua_pop(L, 1);
          break;
        }
        lua_pushliteral(L,
                        "'tostring' must return a string to be concatenated");
        lua_error(L);
    }
  }
  if (!needsToString) {
    lua_pop(L, 1);
  }
  return out.str();
}

void
qlua::pushQVariant(lua_State* L, const QVariant& variant)
{
  QMetaType type = variant.metaType();
  switch (type.id()) {
    case QMetaType::UnknownType:
    case QMetaType::Nullptr:
      lua_pushnil(L);
      return;
    case QMetaType::Bool:
      lua_pushboolean(L, variant.toInt());
      return;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::Short:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
    case QMetaType::UShort:
      push(L, variant.toLongLong());
      return;
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Float16:
      push(L, variant.toDouble());
      return;
    case QMetaType::QChar:
      push(L, variant.toChar());
      return;
    case QMetaType::QString:
      push(L, variant.toString());
      return;
    case QMetaType::QDate:
    case QMetaType::QDateTime:
      push(L, variant.toDateTime().toSecsSinceEpoch());
      return;
    case QMetaType::QByteArray:
      push(L, variant.toByteArray());
      return;
    case QMetaType::Char: {
      const char c = variant.value<char>();
      lua_pushlstring(L, &c, 1);
      return;
    }
    case QMetaType::Char16: {
      const char16_t c = variant.value<char16_t>();
      push(L, QString::fromUtf16(&c, 1));
      return;
    }
    case QMetaType::Char32: {
      const char32_t c = variant.value<char32_t>();
      push(L, QString::fromUcs4(&c, 1));
      return;
    }
    case QMetaType::SChar: {
      const char c = static_cast<char>(variant.value<signed char>());
      lua_pushlstring(L, &c, 1);
      return;
    }
    case QMetaType::UChar: {
      const char c = static_cast<char>(variant.value<unsigned char>());
      lua_pushlstring(L, &c, 1);
      return;
    }
    case QMetaType::QBrush:
      push(L, variant.value<QBrush>().color());
    case QMetaType::QColor:
      push(L, variant.value<QColor>());
      return;
    case QMetaType::QUuid:
      push(L, variant.toUuid().toString());
      return;
    case QMetaType::QStringList:
      pushList(L, variant.toStringList());
      return;
    case QMetaType::QVariantHash:
      pushMap(L, variant.toHash());
      return;
    case QMetaType::QVariantMap:
      pushMap(L, variant.toMap());
      return;
    case QMetaType::QVariantList:
      if (variant.canConvert<QStringList>()) {
        pushList(L, variant.toStringList());
      } else {
        pushList(L, variant.toList());
      }
      return;
    default:
      if (variant.canConvert<qlonglong>()) {
        push(L, variant.toLongLong());
      } else {
        lua_pushnil(L);
      }
      return;
  }
}

template<>
void
qlua::pushAny(lua_State* L, const QVariant& value)
{
  pushQVariant(L, value);
}

template<>
void
qlua::pushAny(lua_State* L,
              QVariant value) // NOLINT(performance-unnecessary-value-param)
{
  pushQVariant(L, value);
}

bool
qlua::copyValue(lua_State* fromL, lua_State* toL, int idx)
{
  switch (lua_type(fromL, idx)) {
    case LUA_TNONE:
      return true;
    case LUA_TNIL:
      lua_pushnil(toL);
      return true;
    case LUA_TBOOLEAN:
      lua_pushboolean(toL, lua_toboolean(fromL, idx));
      return true;
    case LUA_TNUMBER: {
      int isInt;
      const lua_Integer intResult = lua_tointegerx(fromL, idx, &isInt);
      if (isInt == TRUE) {
        lua_pushinteger(toL, intResult);
      } else {
        lua_pushnumber(toL, lua_tonumber(fromL, idx));
      }
      return true;
    }
    case LUA_TSTRING: {
      size_t len;
      const char* s = lua_tolstring(fromL, idx, &len);
      lua_pushlstring(toL, s, len);
    }
      return true;
    default:
      return false;
  }
}

QLine
qlua::getQLine(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2)
{
  return QLine(
    getInt(L, idxX1), getInt(L, idxY1), getInt(L, idxX2), getInt(L, idxY2));
}

QLineF
qlua::getQLineF(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2)
{
  return QLineF(getNumber(L, idxX1),
                getNumber(L, idxY1),
                getNumber(L, idxX2),
                getNumber(L, idxY2));
}

QMargins
qlua::getQMargins(lua_State* L,
                  int idxLeft,
                  int idxTop,
                  int idxRight,
                  int idxBottom)
{
  return QMargins(getInt(L, idxLeft),
                  getInt(L, idxTop),
                  getInt(L, idxRight),
                  getInt(L, idxBottom));
}

QMarginsF
qlua::getQMarginsF(lua_State* L,
                   int idxLeft,
                   int idxTop,
                   int idxRight,
                   int idxBottom)
{
  return QMarginsF(getNumber(L, idxLeft),
                   getNumber(L, idxTop),
                   getNumber(L, idxRight),
                   getNumber(L, idxBottom));
}

QPoint
qlua::getQPoint(lua_State* L, int idxX, int idxY)
{
  return QPoint(getInt(L, idxX), getInt(L, idxY));
}

QPointF
qlua::getQPointF(lua_State* L, int idxX, int idxY)
{
  return QPointF(getNumber(L, idxX), getNumber(L, idxY));
}

QSize
qlua::getQSize(lua_State* L, int idxWidth, int idxHeight)
{
  return QSize(getInt(L, idxWidth), getInt(L, idxHeight));
}

QSizeF
qlua::getQSizeF(lua_State* L, int idxWidth, int idxHeight)
{
  return QSizeF(getNumber(L, idxWidth), getNumber(L, idxHeight));
}

QRect
qlua::getQRect(lua_State* L,
               int idxLeft,
               int idxTop,
               int idxRight,
               int idxBottom)
{
  return QRect(QPoint(getInt(L, idxLeft), getInt(L, idxTop)),
               QPoint(getInt(L, idxRight), getInt(L, idxBottom)));
}

QRectF
qlua::getQRectF(lua_State* L,
                int idxLeft,
                int idxTop,
                int idxRight,
                int idxBottom)
{
  return QRectF(QPointF(getNumber(L, idxLeft), getNumber(L, idxTop)),
                QPointF(getNumber(L, idxRight), getNumber(L, idxBottom)));
}

QTransform
qlua::getQTransform(lua_State* L,
                    int idxM11,
                    int idxM12,
                    int idxM21,
                    int idxM22,
                    int idxDx,
                    int idxDy)
{
  return QTransform(getNumber(L, idxM11),
                    getNumber(L, idxM12),
                    getNumber(L, idxM21),
                    getNumber(L, idxM22),
                    getNumber(L, idxDx),
                    getNumber(L, idxDy));
}

optional<QPen>
qlua::getQPen(lua_State* L, int idxColor, int idxStyle, int idxWidth)
{
  const QColor color = getQColor(L, idxColor);
  const lua_Integer style = getInteger(L, idxStyle);
  const lua_Number width = getNumber(L, idxWidth);
  if (style < 0 || width < 0) [[unlikely]] {
    return nullopt;
  }

  if (style == 0) {
    return QPen(color,
                width,
                Qt::PenStyle::SolidLine,
                Qt::PenCapStyle::RoundCap,
                Qt::PenJoinStyle::RoundJoin);
  }

  const optional<Qt::PenStyle> penStyle = getPenStyle(style);
  const optional<Qt::PenCapStyle> capStyle = getPenCap(style);
  const optional<Qt::PenJoinStyle> joinStyle = getPenJoin(style);
  if (!penStyle || !capStyle || !joinStyle) [[unlikely]] {
    return nullopt;
  }

  return QPen(color, width, *penStyle, *capStyle, *joinStyle);
}

optional<QPolygonF>
qlua::getQPolygonF(lua_State* L, int idx)
{
  const string_view s = getString(L, idx);
  const qsizetype commaCount = std::count(s.cbegin(), s.cend(), ',');
  if (commaCount % 2 == 0 || commaCount < 3) [[unlikely]] {
    return nullopt;
  }
  QList<QPointF> points;
  points.reserve((commaCount + 1) / 2);
  std::istringstream stream((string(s)));
  for (string sX, sY;
       std::getline(stream, sX, ',') && std::getline(stream, sY, ',');) {
    qreal dX = stod(sX);
    qreal dY = stod(sY);
    if (!std::isfinite(dX) || !std::isfinite(dY)) [[unlikely]] {
      return nullopt;
    }
    points.append(QPointF(dX, dY));
  }
  return QPolygonF(points);
}

optional<MiniWindow::ButtonFrame>
qlua::getButtonFrame(lua_State* L,
                     int idx,
                     optional<MiniWindow::ButtonFrame> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer")) {
    return ifNil;
  }
  const MiniWindow::ButtonFrame value =
    static_cast<MiniWindow::ButtonFrame>(toInteger(L, idx));

  if (value == MiniWindow::ButtonFrame::Raised ||
      value == MiniWindow::ButtonFrame::Etched ||
      value == MiniWindow::ButtonFrame::Bump ||
      value == MiniWindow::ButtonFrame::Sunken) {
    return value;
  }
  return nullopt;
}

optional<Qt::BrushStyle>
qlua::getBrush(lua_State* L, int idx, optional<Qt::BrushStyle> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer")) {
    return ifNil;
  }

  switch (static_cast<ScriptBrush>(toInteger(L, idx))) {
    case ScriptBrush::SolidPattern:
      return Qt::BrushStyle::SolidPattern;
    case ScriptBrush::NoBrush:
      return Qt::BrushStyle::NoBrush;
    case ScriptBrush::HorPattern:
      return Qt::BrushStyle::HorPattern;
    case ScriptBrush::VerPattern:
      return Qt::BrushStyle::VerPattern;
    case ScriptBrush::FDiagPattern:
      return Qt::BrushStyle::FDiagPattern;
    case ScriptBrush::BDiagPattern:
      return Qt::BrushStyle::BDiagPattern;
    case ScriptBrush::CrossPattern:
      return Qt::BrushStyle::CrossPattern;
    case ScriptBrush::DiagCrossPattern:
      return Qt::BrushStyle::DiagCrossPattern;
    case ScriptBrush::Dense4Pattern:
      return Qt::BrushStyle::Dense4Pattern;
    case ScriptBrush::Dense2Pattern:
      return Qt::BrushStyle::Dense2Pattern;
    case ScriptBrush::Dense1Pattern:
      return Qt::BrushStyle::Dense1Pattern;
    case ScriptBrush::HorWaves:
      return Qt::BrushStyle::HorPattern;
    case ScriptBrush::VerWaves:
      return Qt::BrushStyle::VerPattern;
    default:
      return nullopt;
  }
}

optional<Qt::CursorShape>
qlua::getCursor(lua_State* L, int idx, optional<Qt::CursorShape> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer")) {
    return ifNil;
  }

  switch (static_cast<ScriptCursor>(toInteger(L, idx))) {
    case ScriptCursor::BlankCursor:
      return Qt::CursorShape::BlankCursor;
    case ScriptCursor::ArrowCursor:
      return Qt::CursorShape::ArrowCursor;
    case ScriptCursor::OpenHandCursor:
      return Qt::CursorShape::OpenHandCursor;
    case ScriptCursor::IBeamCursor:
      return Qt::CursorShape::IBeamCursor;
    case ScriptCursor::CrossCursor:
      return Qt::CursorShape::CrossCursor;
    case ScriptCursor::WaitCursor:
      return Qt::CursorShape::WaitCursor;
    case ScriptCursor::UpArrowCursor:
      return Qt::CursorShape::UpArrowCursor;
    case ScriptCursor::SizeFDiagCursor:
      return Qt::CursorShape::SizeFDiagCursor;
    case ScriptCursor::SizeBDiagCursor:
      return Qt::CursorShape::SizeBDiagCursor;
    case ScriptCursor::SizeHorCursor:
      return Qt::CursorShape::SizeHorCursor;
    case ScriptCursor::SizeVerCursor:
      return Qt::CursorShape::SizeVerCursor;
    case ScriptCursor::SizeAllCursor:
      return Qt::CursorShape::SizeAllCursor;
    case ScriptCursor::ForbiddenCursor:
      return Qt::CursorShape::ForbiddenCursor;
    case ScriptCursor::WhatsThisCursor:
      return Qt::CursorShape::WhatsThisCursor;
    default:
      return nullopt;
  }
}

optional<QFont::StyleHint>
qlua::getFontHint(lua_State* L, int idx, optional<QFont::StyleHint> ifNil)
{
  if (!checkIsSome(L, idx, LUA_TNUMBER, "integer")) {
    return ifNil;
  }

  const lua_Integer style = toInteger(L, idx);
  switch (static_cast<FontPitch>(style & 0xF)) // pitch
  {
    case FontPitch::Default:
    case FontPitch::Fixed:
    case FontPitch::Variable:
      break;
    case FontPitch::Monospace:
      return QFont::StyleHint::Monospace;
    default:
      return nullopt;
  }

  switch (static_cast<FontFamily>(style & ~0xF)) {
    case FontFamily::Roman:
      return QFont::StyleHint::Serif;
    case FontFamily::Swiss:
      return QFont::StyleHint::SansSerif;
    case FontFamily::Modern:
      return QFont::StyleHint::TypeWriter;
    case FontFamily::Script:
      return QFont::StyleHint::Cursive;
    case FontFamily::Decorative:
      return QFont::StyleHint::Decorative;
    default:
      return nullopt;
  }
}
