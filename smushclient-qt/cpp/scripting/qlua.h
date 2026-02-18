#pragma once
#include "../bridge/variableview.h"
#include "miniwindow/miniwindow.h"
#include "rust/cxx.h"
#include "scriptenums.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"
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
getFlags(lua_State* L, int idx)
{
  return static_cast<QFlags<T>>(getInt(L, idx));
}
template<typename T>
inline QFlags<T>
getFlags(lua_State* L, int idx, QFlags<T> ifNil)
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

std::string_view
getString(lua_State* L, int idx);
std::string_view
getString(lua_State* L, int idx, std::string_view ifNil);
std::string_view
toString(lua_State* L, int idx);

QByteArray
concatBytes(lua_State* L);
QString
concatStrings(lua_State* L);

void
pushBool(lua_State* L, bool value);
const char*
pushBytes(lua_State* L, const QByteArray& bytes);
void
pushQColor(lua_State* L, const QColor& color);
void
pushQHash(lua_State* L, const QVariantHash& variants);
void
pushQMap(lua_State* L, const QVariantMap& variants);
const char*
pushQString(lua_State* L, const QString& string);
void
pushQStrings(lua_State* L, const QStringList& strings);
void
pushQVariant(lua_State* L, const QVariant& variant);
void
pushQVariants(lua_State* L, const QVariantList& variants);
const char*
pushRString(lua_State* L, rust::Str string);
const char*
pushString(lua_State* L, std::string_view string);
const char*
pushVariable(lua_State* L, VariableView variable);

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

std::optional<CircleOp>
getCircleOp(lua_State* L, int idx);

std::optional<Qt::CursorShape>
getCursor(lua_State* L,
          int idx,
          std::optional<Qt::CursorShape> ifNil = std::nullopt);

std::optional<ffi::filter::Directions>
getDirections(lua_State* L,
              int idx,
              std::optional<ffi::filter::Directions> ifNil = std::nullopt);

std::optional<MiniWindow::DrawImageMode>
getDrawImageMode(lua_State* L,
                 int idx,
                 std::optional<MiniWindow::DrawImageMode> ifNil = std::nullopt);

std::optional<FilterOp>
getFilterOp(lua_State* L,
            int idx,
            std::optional<FilterOp> ifNil = std::nullopt);

std::optional<QFont::StyleHint>
getFontHint(lua_State* L,
            int idx,
            std::optional<QFont::StyleHint> ifNil = std::nullopt);

std::optional<Qt::Orientation>
getOrientation(lua_State* L,
               int idx,
               std::optional<Qt::Orientation> ifNil = std::nullopt);

std::optional<RectOp>
getRectOp(lua_State* L, int idx);

std::optional<SendTarget>
getSendTarget(lua_State* L,
              int idx,
              std::optional<SendTarget> ifNil = std::nullopt);

std::optional<MiniWindow::Position>
getWindowPosition(lua_State* L,
                  int idx,
                  std::optional<MiniWindow::Position> ifNil = std::nullopt);
} // namespace qlua
