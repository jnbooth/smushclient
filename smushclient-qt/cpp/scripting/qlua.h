#pragma once
#include "miniwindow/miniwindow.h"
#include "rust/cxx.h"
#include "scriptenums.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"
#include <limits>
extern "C"
{
#include "lua.h"
}

enum class SendTarget;

namespace qlua {
constexpr QColor
rgbCodeToColor(lua_Integer rgb) noexcept
{
  if (rgb == -1 || rgb > 0xFFFFFF) [[unlikely]] {
    return QColor();
  }
  const int code = static_cast<int>(rgb);
  return QColor(code & 0xFF, (code >> 8) & 0xFF, (code >> 16) & 0xFF);
}

inline lua_Integer
colorToRgbCode(const QColor& color)
{
  if (!color.isValid()) [[unlikely]] {
    return -1;
  }
  int r, g, b;
  color.getRgb(&r, &g, &b);
  return b << 16 | g << 8 | r;
}

int
expectMaxArgs(lua_State* L, int max);

bool
getBool(lua_State* L, int idx);
bool
getBool(lua_State* L, int idx, bool ifNil);

QByteArrayView
getBytes(lua_State* L, int idx);
lua_Integer
getInteger(lua_State* L, int idx);
lua_Integer
getInteger(lua_State* L, int idx, lua_Integer ifNil);

int
getInt(lua_State* L, int idx);
int
getInt(lua_State* L, int idx, int ifNil);

lua_Integer
getIntegerOrBool(lua_State* L, int idx);
lua_Integer
getIntegerOrBool(lua_State* L, int idx, lua_Integer ifNil);

lua_Number
getNumber(lua_State* L, int idx);
lua_Number
getNumber(lua_State* L, int idx, lua_Number ifNil);

QColor
getQColor(lua_State* L, int idx);
QColor
getQColor(lua_State* L, int idx, const QColor& ifNil);
QColor
getCustomColor(lua_State* L, int idx);

template<typename T>
inline QFlags<T>
getQFlags(lua_State* L, int idx)
{
  return static_cast<QFlags<T>>(getInt(L, idx));
}
template<typename T>
inline QFlags<T>
getQFlags(lua_State* L, int idx, QFlags<T> ifNil)
{
  return static_cast<QFlags<T>>(getInt(L, idx, static_cast<int>(ifNil)));
}

QString
getQString(lua_State* L, int idx);
QString
getQString(lua_State* L, int idx, const QString& ifNil);

QVariant
getQVariant(lua_State* L, int idx, int type);
QVariant
getQVariant(lua_State* L, int idx);

std::optional<std::string_view>
getScriptName(lua_State* L, int idx);

template<typename T, T MIN = enum_bounds<T>::min, T MAX = enum_bounds<T>::max>
std::optional<T>
getEnum(lua_State* L, int idx, std::optional<T> ifNil = std::nullopt)
{
  constexpr lua_Integer ifNilInt = std::numeric_limits<lua_Integer>::min();
  const lua_Integer underlying = getInteger(L, idx, ifNilInt);
  if (underlying == ifNilInt) {
    return ifNil;
  }
  if (underlying < static_cast<lua_Integer>(MIN) ||
      underlying > static_cast<lua_Integer>(MAX)) [[unlikely]] {
    return std::nullopt;
  }
  return static_cast<T>(underlying);
}

template<typename T, T MIN = enum_bounds<T>::min, T MAX = enum_bounds<T>::max>
std::optional<T>
getEnum(lua_State* L, int idx, T ifNil)
{
  return getEnum(L, idx, std::optional(ifNil));
}

std::string_view
getString(lua_State* L, int idx);
std::string_view
getString(lua_State* L, int idx, std::string_view ifNil);
std::string_view
toString(lua_State* L, int idx);

QByteArray
concatBytes(lua_State* L);
std::string
concatStrings(lua_State* L);

bool
copyValue(lua_State* fromL, lua_State* toL, int idx);

QLine
getQLine(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2);
QLineF
getQLineF(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2);
QMargins
getQMargins(lua_State* L, int idxLeft, int idxTop, int idxRight, int idxBottom);
QMarginsF
getQMarginsF(lua_State* L,
             int idxLeft,
             int idxTop,
             int idxRight,
             int idxBottom);
QPoint
getQPoint(lua_State* L, int idxX, int idxY);
QPointF
getQPointF(lua_State* L, int idxX, int idxY);
QSize
getQSize(lua_State* L, int idxWidth, int idxHeight);
QSizeF
getQSizeF(lua_State* L, int idxWidth, int idxHeight);
QRect
getQRect(lua_State* L, int idxLeft, int idxTop, int idxRight, int idxBottom);
QRectF
getQRectF(lua_State* L, int idxLeft, int idxTop, int idxRight, int idxBottom);
QTransform
getQTransform(lua_State* L,
              int idxM11,
              int idxM12,
              int idxM21,
              int idxM22,
              int idxDx,
              int idxDy);

std::optional<QPen>
getQPen(lua_State* L, int idxColor, int idxStyle, int idxWidth);

std::optional<QPolygonF>
getQPolygonF(lua_State* L, int idx);

std::optional<BlendMode>
getBlendMode(lua_State* L,
             int idx,
             std::optional<BlendMode> ifNil = std::nullopt);

std::optional<Qt::BrushStyle>
getBrush(lua_State* L,
         int idx,
         std::optional<Qt::BrushStyle> ifNil = std::nullopt);

std::optional<MiniWindow::ButtonFrame>
getButtonFrame(lua_State* L,
               int idx,
               std::optional<MiniWindow::ButtonFrame> ifNil = std::nullopt);

std::optional<Qt::CursorShape>
getCursor(lua_State* L,
          int idx,
          std::optional<Qt::CursorShape> ifNil = std::nullopt);

std::optional<QFont::StyleHint>
getFontHint(lua_State* L,
            int idx,
            std::optional<QFont::StyleHint> ifNil = std::nullopt);

void
pushQVariant(lua_State* L, const QVariant& variant);

#define PUSH(T, f)                                                             \
  inline void push(lua_State* L, T value)                                      \
  {                                                                            \
    f(L, value);                                                               \
  }

PUSH(const char*, lua_pushstring);
PUSH(lua_Integer, lua_pushinteger);
PUSH(lua_Number, lua_pushnumber);

#undef PUSH

#define NPUSH(T)                                                               \
  inline void push(lua_State* L, T value)                                      \
  {                                                                            \
    lua_pushinteger(L, static_cast<lua_Integer>(value));                       \
  }

NPUSH(ApiCode);
NPUSH(AliasFlag);
NPUSH(OperatingSystem);
NPUSH(SendTarget);
NPUSH(TimerFlag);
NPUSH(TriggerFlag);
NPUSH(int);
NPUSH(size_t);

#undef NPUSH

#define SPUSH(T)                                                               \
  inline const char* push(lua_State* L, T value)                               \
  {                                                                            \
    return lua_pushlstring(L, value.data(), value.size());                     \
  }

SPUSH(const QByteArray&);
SPUSH(rust::Str);
SPUSH(const std::string&);
SPUSH(std::string_view);

#undef SPUSH

inline void
push(lua_State* L, bool value)
{
  lua_pushboolean(L, static_cast<int>(value));
}

inline void
push(lua_State* L, const QColor& color)
{
  push(L, colorToRgbCode(color));
}

inline const char*
push(lua_State* L, const QString& string)
{
  return push(L, string.toUtf8());
}

inline const char*
push(lua_State* L, QChar ch)
{
  return push(L, QString(ch));
}

template<typename T>
void
pushAny(lua_State* L, T value)
{
  push(L, value);
}

template<>
void
pushAny(lua_State* L, const QVariant& value);

template<>
void
pushAny(lua_State* L, QVariant value);

template<typename T>
void
pushList(lua_State* L, const T& list)
{
  lua_createtable(L, static_cast<int>(list.size()), 0);
  lua_Integer i = 1;
  for (const auto& item : list) {
    pushAny(L, item);
    lua_rawseti(L, -2, i);
    ++i;
  }
}

template<typename T>
void
pushMap(lua_State* L, const T& map)
{
  lua_createtable(L, 0, static_cast<int>(map.size()));
  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    push(L, it.key());
    pushAny(L, it.value());
    lua_rawset(L, -3);
  }
}

template<typename K, typename V>
void
pushEntry(lua_State* L, K key, V value, int idx = -1)
{
  push(L, key);
  push(L, value);
  lua_rawset(L, idx < 0 ? idx - 2 : idx);
}

template<typename V>
void
pushEntry(lua_State* L, const char* key, V value, int idx = -1)
{
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key);
}

template<typename V>
void
pushEntry(lua_State* L, const std::string& key, V value, int idx = -1)
{
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key.c_str());
}

template<typename V>
void
pushEntry(lua_State* L, const QByteArray& key, V value, int idx = -1)
{
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key.toStdString().c_str());
}

template<typename V>
void
pushEntry(lua_State* L, const QString& key, V value, int idx = -1)
{
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key.toStdString().c_str());
}

} // namespace qlua
