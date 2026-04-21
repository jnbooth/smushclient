#pragma once
#include "../casting.h"
#include "../enumbounds.h"
#include "rust/cxx.h"
#include "smushclient_qt/views.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QUuid>
#include <QtGui/QPen>
#include <QtNetwork/QHostAddress>
#include <type_traits>
extern "C"
{
#include "lua.h"
}

inline bool
lua_tobool(lua_State* L, int idx)
{
  return static_cast<bool>(lua_toboolean(L, idx));
}

inline QByteArrayView
lua_tobytes(lua_State* L, int idx)
{
  size_t len;
  return { lua_tolstring(L, idx, &len), static_cast<qsizetype>(len) };
}

inline std::string_view
lua_tostr(lua_State* L, int idx)
{
  size_t len;
  return { lua_tolstring(L, idx, &len), len };
}

namespace qlua {
using std::nullopt;
using std::optional;

QColor
rgbCodeToColor(lua_Integer rgb) noexcept;

lua_Integer
colorToRgbCode(const QColor& color);

int
throwTooManyArgsError(lua_State* L, int max);

QByteArray
concatArgs(lua_State* L, int startIdx = 1, QByteArrayView delim = {});

bool
copyValue(lua_State* fromL, lua_State* toL, int idx);

inline int
expectMaxArgs(lua_State* L, int max)
{
  if (int n = lua_gettop(L); n <= max) [[likely]] {
    return n;
  }
  return throwTooManyArgsError(L, max);
}

bool
getBool(lua_State* L, int idx, optional<bool> ifNil = nullopt);

QByteArrayView
getBytes(lua_State* L, int idx, optional<QByteArrayView> ifNil = nullopt);

int
getInt(lua_State* L, int idx, optional<int> ifNil = nullopt);

lua_Integer
getInteger(lua_State* L, int idx, optional<lua_Integer> ifNil = nullopt);

lua_Integer
getOption(lua_State* L, int idx);

lua_Number
getNumber(lua_State* L, int idx, optional<lua_Number> ifNil = nullopt);

optional<Qt::BrushStyle>
getBrush(lua_State* L, int idx, optional<Qt::BrushStyle> ifNil = nullopt);

optional<Qt::CursorShape>
getCursor(lua_State* L, int idx, optional<Qt::CursorShape> ifNil = nullopt);

QColor
getCustomColor(lua_State* L, int idx, optional<QColor> ifNil = nullopt);

template<typename T>
optional<T>
getEnum(lua_State* L, int idx)
  requires(std::is_enum_v<T>)
{
  return enum_cast<T>(getInteger(L, idx));
}

template<typename T>
optional<T>
getEnum(lua_State* L, int idx, T ifNil)
  requires(std::is_enum_v<T>)
{
  return enum_cast<T>(getInteger(L, idx, static_cast<lua_Integer>(ifNil)));
}

optional<QBrush>
getQBrush(lua_State* L,
          int idxColor,
          int idxStyle,
          optional<Qt::BrushStyle> ifNil = nullopt);

QColor
getQColor(lua_State* L, int idx, optional<QColor> ifNil = nullopt);

template<typename T>
inline QFlags<T>
getQFlags(lua_State* L, int idx)
{
  return QFlags<T>::fromInt(getInt(L, idx));
}

template<typename T>
inline QFlags<T>
getQFlags(lua_State* L, int idx, QFlags<T> ifNil)
{
  return QFlags<T>::fromInt(getInt(L, idx, ifNil.toInt()));
}

QFont
getQFont(lua_State* L, int idxFamily);
QFont
getQFont(lua_State* L, int idxFamily, int idxSize);
QFont
getQFont(lua_State* L, int idxFamily, int idxSize, int idxStyle);

QLine
getQLine(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2);

QLineF
getQLineF(lua_State* L, int idxX1, int idxY1, int idxX2, int idxY2);

optional<QPen>
getQPen(lua_State* L, int idxColor, int idxStyle, int idxWidth);

QPoint
getQPoint(lua_State* L, int idxX, int idxY);

QPointF
getQPointF(lua_State* L, int idxX, int idxY);

optional<QPolygonF>
getQPolygonF(lua_State* L, int idx);

QSize
getQSize(lua_State* L, int idxWidth, int idxHeight);

QSizeF
getQSizeF(lua_State* L, int idxWidth, int idxHeight);

QRect
getQRect(lua_State* L, int idxLeft, int idxTop, int idxRight, int idxBottom);

QRectF
getQRectF(lua_State* L, int idxLeft, int idxTop, int idxRight, int idxBottom);

QString
getQString(lua_State* L, int idx);
QString
getQString(lua_State* L, int idx, const QString& ifNil);

std::string_view
getString(lua_State* L, int idx, optional<std::string_view> ifNil = nullopt);

bool
isScriptName(lua_State* L, std::string_view name);

template<typename T>
void
push(lua_State* L, T value) = delete;

template<typename T>
void
push(lua_State* L, T value)
  requires(std::is_enum_v<T>)
{
  lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template<typename T>
void
push(lua_State* L, QFlags<T> flags)
{
  lua_pushinteger(L, flags.toInt());
}

void
push(lua_State* L, const QVariant& value);

inline void
push(lua_State* L, const char* value)
{
  lua_pushstring(L, value);
}

inline void
push(lua_State* L, char ch)
{
  lua_pushlstring(L, &ch, 1);
}

void
push(lua_State* L, char16_t ch);

void
push(lua_State* L, char32_t ch);

void
push(lua_State* L, const QRect& rect);

#define IMPL_PUSH(T)                                                           \
  inline void push(lua_State* L, T value)                                      \
  {                                                                            \
    lua_pushlstring(L, value.data(), value.size());                            \
  }

IMPL_PUSH(QByteArrayView);
IMPL_PUSH(const QByteArray&);
IMPL_PUSH(const rust::String&);
IMPL_PUSH(rust::Str);
IMPL_PUSH(const std::string&);
IMPL_PUSH(std::string_view);
IMPL_PUSH(rust::variable_view);

#undef IMPL_PUSH
#define IMPL_PUSH(T, f, expr)                                                  \
  inline void push(lua_State* L, T value)                                      \
  {                                                                            \
    (f)(L, (expr));                                                            \
  }

IMPL_PUSH(lua_Integer, lua_pushinteger, value);
IMPL_PUSH(lua_Number, lua_pushnumber, value);
IMPL_PUSH(bool, lua_pushboolean, value);
IMPL_PUSH(int, lua_pushinteger, value);
IMPL_PUSH(uint16_t, lua_pushinteger, value);
IMPL_PUSH(uint32_t, lua_pushinteger, value);
IMPL_PUSH(size_t, lua_pushinteger, clamped_cast<lua_Integer>(value));
IMPL_PUSH(const QColor&, lua_pushinteger, colorToRgbCode(value));
IMPL_PUSH(signed char, push, static_cast<char>(value));
IMPL_PUSH(unsigned char, push, static_cast<char>(value));
IMPL_PUSH(QChar, push, value.unicode());
IMPL_PUSH(const QString&, push, value.toUtf8());
IMPL_PUSH(const QDateTime&, lua_pushinteger, value.toSecsSinceEpoch());
IMPL_PUSH(const QHostAddress&, push, value.toString());
IMPL_PUSH(const QUuid&, push, value.toByteArray(QUuid::WithoutBraces));

#undef IMPL_PUSH

template<typename T>
concept Pushable = requires(lua_State* L, T t) { qlua::push(L, t); };

template<typename T>
void
pushList(lua_State* L, const T& list)
  requires(Pushable<typename T::value_type>)
{
  lua_createtable(L, static_cast<int>(list.size()), 0);
  lua_Integer i = 0;
  for (const auto& item : list) {
    push(L, item);
    lua_rawseti(L, -2, ++i);
  }
}

template<Pushable T, size_t N>
void
pushList(lua_State* L, const T (&list)[N])
{
  lua_createtable(L, N, 0);
  lua_Integer i = 0;
  for (const auto& item : list) {
    push(L, item);
    lua_rawseti(L, -2, ++i);
  }
}

template<Pushable K, Pushable V>
void
pushEntry(lua_State* L, K key, V value, int idx = -1)
{
  push(L, key);
  push(L, value);
  lua_rawset(L, idx < 0 ? idx - 2 : idx);
}

template<Pushable V>
void
pushEntry(lua_State* L, const char* key, V value, int idx = -1)
{
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key);
}

template<typename T>
void
pushEntries(lua_State* L, const T& entries)
{
  for (const auto& [key, value] : entries) {
    pushEntry(L, key, value);
  }
}

template<typename T, size_t N>
void
pushMap(lua_State* L, const T (&entries)[N])
{
  lua_createtable(L, 0, N);
  for (const auto& [key, value] : entries) {
    pushEntry(L, key, value);
  }
}

template<typename T>
void
pushMap(lua_State* L, const T& entries)
{
  lua_createtable(L, 0, static_cast<int>(entries.size()));
  pushEntries(L, entries);
}

template<typename T>
void
pushMap(lua_State* L, const T& map)
  requires(requires(T t) { t.asKeyValueRange(); })
{
  lua_createtable(L, 0, static_cast<int>(map.size()));
  pushEntries(L, map.asKeyValueRange());
}
} // namespace qlua
