#pragma once
#include "../enumbounds.h"
#include "rust/cxx.h"
#include "smushclient_qt/views.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QUuid>
#include <QtGui/QPen>
#include <QtNetwork/QHostAddress>
#include <type_traits>
extern "C"
{
#include "lua.h"
}

namespace qlua {
using std::nullopt;
using std::optional;

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

QByteArray
concatArgs(lua_State* L, int startIdx = 1, QByteArrayView delim = {});

bool
copyValue(lua_State* fromL, lua_State* toL, int idx);

int
expectMaxArgs(lua_State* L, int max);

bool
getBool(lua_State* L, int idx, optional<bool> ifNil = nullopt);

QByteArrayView
getBytes(lua_State* L, int idx, optional<QByteArrayView> ifNil = nullopt);

int
getInt(lua_State* L, int idx, optional<int> ifNil = nullopt);

lua_Integer
getInteger(lua_State* L, int idx, optional<lua_Integer> ifNil = nullopt);

lua_Integer
getIntegerOrBool(lua_State* L, int idx, optional<lua_Integer> ifNil = nullopt);

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
{
  return enum_cast<T>(getInteger(L, idx));
}
template<typename T>
optional<T>
getEnum(lua_State* L, int idx, T ifNil)
{
  const lua_Integer nIfNil = static_cast<lua_Integer>(ifNil);
  const lua_Integer underlying = getInteger(L, idx, nIfNil);
  return (underlying == nIfNil) ? ifNil : enum_cast<T>(underlying);
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
getQFont(lua_State* L, int idx);
QFont
getQFont(lua_State* L, int idx, const QString& ifNil);

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

QTransform
getQTransform(lua_State* L,
              int idxM11,
              int idxM12,
              int idxM21,
              int idxM22,
              int idxDx,
              int idxDy);

QVariant
getQVariant(lua_State* L, int idx);

std::string_view
getString(lua_State* L, int idx, optional<std::string_view> ifNil = nullopt);

std::string_view
toString(lua_State* L, int idx);

bool
isScriptName(lua_State* L, std::string_view name);

void
pushQVariant(lua_State* L, const QVariant& variant);

template<typename T>
void
push(lua_State* L, T value)
  requires(std::is_enum_v<T>)
{
  lua_pushinteger(L, static_cast<lua_Integer>(value));
}

inline const char*
push(lua_State* L, const char* value)
{
  return lua_pushstring(L, value);
}

void
push(lua_State* L, const QRect& rect);
void
push(lua_State* L, const QUuid& id);

#define IMPL_PUSH(T, f, op)                                                    \
  inline void push(lua_State* L, T value)                                      \
  {                                                                            \
    f(L, op(value));                                                           \
  }

IMPL_PUSH(lua_Integer, lua_pushinteger, );
IMPL_PUSH(lua_Number, lua_pushnumber, );
IMPL_PUSH(bool, lua_pushboolean, static_cast<int>);
IMPL_PUSH(int, lua_pushinteger, static_cast<lua_Integer>);
IMPL_PUSH(size_t, lua_pushinteger, static_cast<lua_Integer>);
IMPL_PUSH(uint32_t, lua_pushinteger, static_cast<lua_Integer>);
IMPL_PUSH(const QColor&, lua_pushinteger, colorToRgbCode);

#undef IMPL_PUSH
#define IMPL_PUSH(T)                                                           \
  inline const char* push(lua_State* L, T value)                               \
  {                                                                            \
    return lua_pushlstring(L, value.data(), value.size());                     \
  }

IMPL_PUSH(const QByteArray&);
IMPL_PUSH(const rust::String&);
IMPL_PUSH(rust::Str);
IMPL_PUSH(const std::string&);
IMPL_PUSH(std::string_view);
IMPL_PUSH(rust::variable_view);

#undef IMPL_PUSH
#define IMPL_PUSH(T, op)                                                       \
  inline const char* push(lua_State* L, T value)                               \
  {                                                                            \
    return push(L, value.op);                                                  \
  }

IMPL_PUSH(const QString&, toUtf8());
IMPL_PUSH(const QHostAddress&, toString());

#undef IMPL_PUSH

inline const char*
push(lua_State* L, QChar ch)
{
  if (const char16_t code = ch.unicode(); code <= 127) [[likely]] {
    const char ccode = static_cast<char>(code);
    return lua_pushlstring(L, &ccode, 1);
  }
  return push(L, QString(ch));
}

template<typename T>
void
pushList(lua_State* L, const T& list)
{
  lua_createtable(L, static_cast<int>(list.size()), 0);
  lua_Integer i = 0;
  for (const auto& item : list) {
    push(L, item);
    lua_rawseti(L, -2, ++i);
  }
}

template<>
void
pushList(lua_State* L, const QList<QVariant>& list);

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
  for (const char& c : key) {
    if (c == '\0') {
      push(L, key);
      push(L, value);
      lua_rawset(L, idx < 0 ? idx - 2 : idx);
      return;
    }
  }
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key.c_str());
}

template<typename V>
void
pushEntry(lua_State* L, const QByteArray& key, V value, int idx = -1)
{
  for (const char& c : key) {
    if (c == '\0') {
      push(L, key);
      push(L, value);
      lua_rawset(L, idx < 0 ? idx - 2 : idx);
      return;
    }
  }
  push(L, value);
  lua_setfield(L, idx < 0 ? idx - 1 : idx, key.data());
}

template<typename V>
void
pushEntry(lua_State* L, const QString& key, V value, int idx = -1)
{
  pushEntry(L, key.toUtf8(), value, idx);
}

template<typename T>
void
pushMap(lua_State* L, const T& map)
{
  lua_createtable(L, 0, static_cast<int>(map.size()));
  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    pushEntry(L, it.key(), it.value());
  }
}

template<typename K>
void
pushMap(lua_State* L, const QHash<K, QVariant>& map)
{
  lua_createtable(L, 0, static_cast<int>(map.size()));
  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    push(L, it.key());
    pushQVariant(L, it.value());
    lua_rawset(L, -3);
  }
}

template<typename K>
void
pushMap(lua_State* L, const QMap<K, QVariant>& map)
{
  lua_createtable(L, 0, static_cast<int>(map.size()));
  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    push(L, it.key());
    pushQVariant(L, it.value());
    lua_rawset(L, -3);
  }
}

template<typename T>
void
pushMap(lua_State* L, const rust::Vec<T>& entries)
{
  lua_createtable(L, 0, static_cast<int>(entries.size()));
  for (const auto& entry : entries) {
    pushEntry(L, entry.key, entry.value);
  }
}

} // namespace qlua
