#include "luaapi.h"
#include <QtCore/QPointer>
#include "qlua.h"
#include "scriptapi.h"
#include "scriptenums.h"
#include "worldproperties.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define API_REG_KEY "smushclient.api"
#define INDEX_REG_KEY "smushclient.plugin"
#define WORLD_LIB_KEY "world"
#define WORLD_REG_KEY "smushclient.world"

using std::optional;
using std::string;
using std::string_view;
using stringmap = std::unordered_map<string, string>;

// localization

QString fmtNoSuchPlugin(const QString &id)
{
  return ScriptApi::tr("Plugin ID (%1) is not installed")
      .arg(id);
}

QString fmtPluginDisabled(const Plugin &plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) is not enabled")
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtSelfCall(const Plugin &plugin)
{
  return ScriptApi::tr("Plugin '%1' (%2) attempted to call itself via CallPlugin")
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtNoSuchRoutine(const Plugin &plugin, string_view routine)
{
  return ScriptApi::tr("No function '%1' in plugin '%2' (%3)")
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtBadParam(int idx, const char *type)
{
  return ScriptApi::tr("Cannot pass argument #%1 (%2 type) to CallPlugin")
      .arg(idx)
      .arg(QString::fromUtf8(type));
}

QString fmtCallError(const Plugin &plugin, string_view routine)
{
  return ScriptApi::tr("Runtime error in function '%1', plugin '%2' (%3)")
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(plugin.name())
      .arg(plugin.id());
}

QString fmtBadReturn(const Plugin &plugin, string_view routine, int idx, const char *type)
{
  return ScriptApi::tr("Cannot handle return value #%1 (%2 type) from function '%3' in plugin '%4' (%5)")
      .arg(idx)
      .arg(QString::fromUtf8(type))
      .arg(QString::fromUtf8(routine.data(), routine.size()))
      .arg(plugin.name())
      .arg(plugin.id());
}

// utils

inline int returnCode(lua_State *L, ApiCode code)
{
  lua_pushinteger(L, (lua_Integer)code);
  return 1;
}

inline int returnCode(lua_State *L, ApiCode code, const QString &reason)
{
  lua_pushinteger(L, (lua_Integer)code);
  qlua::pushQString(L, reason);
  return 2;
}

template <typename T>
inline T *createUserdata(lua_State *L, const char *k)
{
  void *ud = lua_newuserdata(L, sizeof(T));
  lua_setfield(L, LUA_REGISTRYINDEX, k);
  return static_cast<T *>(ud);
}

template <typename T>
inline T *getUserdata(lua_State *L, const char *k)
{
  lua_getfield(L, LUA_REGISTRYINDEX, k);
  void *ud = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return static_cast<T *>(ud);
}

void returnError(lua_State *L, const char *e)
{
  lua_pushstring(L, e);
  lua_error(L);
}

void setLuaApi(lua_State *L, ScriptApi *api)
{
  *createUserdata<QPointer<ScriptApi>>(L, API_REG_KEY) = api;
}

inline ScriptApi &getApi(lua_State *L)
{
  QPointer<ScriptApi> *ud = getUserdata<QPointer<ScriptApi>>(L, API_REG_KEY);
  if (ud == nullptr || ud->isNull())
    returnError(L, "Userdata was deleted");
  return *ud->data();
}

void setPluginIndex(lua_State *L, size_t index)
{
  lua_pushinteger(L, index);
  lua_setfield(L, LUA_REGISTRYINDEX, INDEX_REG_KEY);
}

inline size_t getPluginIndex(lua_State *L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, INDEX_REG_KEY);
  const size_t index = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return index;
}

// options

static int L_GetAlphaOption(lua_State *L)
{
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  QString value = option.toString();
  if (value.isEmpty() && option.canConvert<QColor>())
    value = option.value<QColor>().name();
  qlua::pushQString(L, value);
  return 1;
}

static int L_GetAlphaOptionList(lua_State *L)
{
  qlua::pushStrings(L, WorldProperties::stringOptions());
  return 1;
}

static int L_GetCurrentValue(lua_State *L)
{
  qlua::pushQVariant(L, getApi(L).GetOption(qlua::getString(L, 1)));
  return 1;
}

static int L_GetOption(lua_State *L)
{
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  const QMetaType type = option.metaType();
  switch (type.id())
  {
  case QMetaType::Double:
  case QMetaType::Float:
  case QMetaType::Float16:
    lua_pushnumber(L, option.value<lua_Number>());
    break;
  case QMetaType::QColor:
    lua_pushinteger(L, qlua::colorToRgbCode(option.value<QColor>()));
    break;
  default:
    if (option.canConvert<lua_Integer>())
      lua_pushinteger(L, option.value<lua_Integer>());
    else
      lua_pushinteger(L, -1);
  }
  return 1;
}

static int L_GetOptionList(lua_State *L)
{
  qlua::pushStrings(L, WorldProperties::numericOptions());
  return 1;
}

static int L_SetAlphaOption(lua_State *L)
{
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getQString(L, 2));
  return 1;
}

static int L_SetOption(lua_State *L)
{
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getNumber(L, 2));
  return 1;
}

// output

inline void insertTextTriples(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3)
    api.ColourTell(
        qlua::getQColor(L, i),
        qlua::getQColor(L, i + 1),
        qlua::getQString(L, i + 2));
}

inline void insertTexts(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; ++i)
    api.Tell(qlua::getQString(L, i));
}

static int L_ColourNameToRGB(lua_State *L)
{
  lua_pushinteger(L, qlua::colorToRgbCode(qlua::getQColor(L, 1)));
  return 1;
}

static int L_ColourNote(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTextTriples(L, api);
  api.finishNote();
  return 0;
}

static int L_ColourTell(lua_State *L)
{
  insertTextTriples(L, getApi(L));
  return 0;
}

static int L_Hyperlink(lua_State *L)
{
  getApi(L).Hyperlink(
      qlua::getQString(L, 1),
      qlua::getQString(L, 2),
      qlua::getQString(L, 3),
      qlua::getQColor(L, 4),
      qlua::getQColor(L, 5),
      qlua::getBool(L, 6),
      qlua::getBool(L, 7, false));
  return 0;
}

static int L_Note(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTexts(L, api);
  api.finishNote();
  return 0;
}

static int L_RGBColourToName(lua_State *L)
{
  qlua::pushQColor(L, qlua::rgbCodeToColor(qlua::getInt(L, 1)));
  return 1;
}

static int L_Send(lua_State *L)
{
  return returnCode(L, getApi(L).Send(qlua::borrowBytes(L, 1)));
}

static int L_SendNoEcho(lua_State *L)
{
  return returnCode(L, getApi(L).SendNoEcho(qlua::borrowBytes(L, 1)));
}

static int L_SetClipboard(lua_State *L)
{
  ScriptApi::SetClipboard(qlua::getQString(L, 1));
  return 0;
}

static int L_Tell(lua_State *L)
{
  insertTexts(L, getApi(L));
  return 0;
}

// plugins

static int L_CallPlugin(lua_State *L)
{
  const Plugin *pluginRef = getApi(L).getPlugin(qlua::getString(L, 1));
  if (pluginRef == nullptr)
    return returnCode(
        L,
        ApiCode::NoSuchPlugin,
        fmtNoSuchPlugin(qlua::getQString(L, 1)));

  const Plugin &plugin = *pluginRef;
  if (plugin.disabled())
    return returnCode(L, ApiCode::PluginDisabled, fmtPluginDisabled(plugin));

  const string_view routine = qlua::getString(L, 2);

  lua_State *L2 = plugin.state();
  if (L2 == L)
    return returnCode(
        L,
        ApiCode::ErrorCallingPluginRoutine, fmtSelfCall(plugin));

  const int n = lua_gettop(L);
  luaL_checkstack(L2, n - 1, nullptr);

  if (lua_getglobal(L2, routine.data()) != LUA_TFUNCTION)
    return returnCode(L, ApiCode::NoSuchRoutine, fmtNoSuchRoutine(plugin, routine));

  const int topBefore = lua_gettop(L2) - 1;

  for (int i = 3; i <= n; ++i)
    if (!qlua::copyValue(L, L2, i))
    {
      lua_settop(L2, topBefore);
      lua_settop(L, 0);
      return returnCode(L, ApiCode::BadParameter, fmtBadParam(i - 2, luaL_typename(L, i)));
    }

  if (lua_pcall(L2, n, LUA_MULTRET, 0) != LUA_OK)
  {
    lua_settop(L, 0);
    lua_pushinteger(L, (lua_Integer)ApiCode::ErrorCallingPluginRoutine);
    qlua::pushQString(L, fmtCallError(plugin, routine));
    size_t size;
    lua_pushlstring(L, lua_tolstring(L2, -1, &size), size);
    lua_settop(L2, topBefore);
    return 3;
  }

  const int topAfter = lua_gettop(L2);
  const int nresults = topAfter - topBefore + 1;
  lua_settop(L, 0);
  luaL_checkstack(L, nresults, nullptr);
  lua_pushinteger(L, (lua_Integer)ApiCode::OK);
  for (int i = topBefore + 1; i <= topAfter; ++i)
  {
    if (!qlua::copyValue(L, L2, i))
      return returnCode(L, ApiCode::ErrorCallingPluginRoutine,
                        fmtBadReturn(plugin, routine, i - topBefore, luaL_typename(L, i)));
  }
  lua_settop(L2, topBefore);
  return nresults;
}

static int L_GetPluginInfo(lua_State *L)
{
  const string_view pluginID = qlua::getString(L, 1);
  const lua_Integer infoType = qlua::getInt(L, 2);
  if (infoType > UINT8_MAX)
  {
    lua_pushnil(L);
    return 1;
  }
  qlua::pushQVariant(L, getApi(L).GetPluginInfo(pluginID, (uint8_t)infoType));
  return 1;
}

// senders

static int L_EnableAlias(lua_State *L)
{
  return returnCode(L, getApi(L).EnableAlias(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableAliasGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableAliasGroup(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnablePlugin(lua_State *L)
{
  return returnCode(L, getApi(L).EnablePlugin(qlua::getString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTimer(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTimer(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTimerGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTimerGroup(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTrigger(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTrigger(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_EnableTriggerGroup(lua_State *L)
{
  return returnCode(L, getApi(L).EnableTrigger(qlua::getQString(L, 1), qlua::getBool(L, 2)));
}

static int L_IsAlias(lua_State *L)
{
  return returnCode(L, getApi(L).IsAlias(qlua::getQString(L, 1)));
}

static int L_IsTimer(lua_State *L)
{
  return returnCode(L, getApi(L).IsTimer(qlua::getQString(L, 1)));
}

static int L_IsTrigger(lua_State *L)
{
  return returnCode(L, getApi(L).IsTrigger(qlua::getQString(L, 1)));
}

// variables

static int L_GetVariable(lua_State *L)
{
  optional<string_view> var = getApi(L).GetVariable(getPluginIndex(L), qlua::getString(L, 1));
  if (!var)
    lua_pushnil(L);
  else
    qlua::pushString(L, *var);
  return 1;
}

static int L_GetPluginVariable(lua_State *L)
{
  optional<string_view> var = getApi(L).GetVariable(qlua::getString(L, 1), qlua::getString(L, 2));
  if (!var)
    lua_pushnil(L);
  else
    qlua::pushString(L, *var);
  return 1;
}

static int L_SetVariable(lua_State *L)
{
  getApi(L).SetVariable(getPluginIndex(L), qlua::getString(L, 1), qlua::getString(L, 2));
  return returnCode(L, ApiCode::OK);
}

// windows

static int L_Redraw(lua_State *)
{
  return 0;
}

static int L_Repaint(lua_State *)
{
  return 0;
}

static int L_TextRectangle(lua_State *L)
{
  const QMargins margins(
      qlua::getInt(L, 1),
      qlua::getInt(L, 2),
      qlua::getInt(L, 3),
      qlua::getInt(L, 4));
  const int offset = qlua::getInt(L, 5);
  const QColor borderColor = qlua::getQColor(L, 6);
  const int borderWidth = qlua::getInt(L, 7);
  const QColor outsideColor = qlua::getQColor(L, 8);
  const optional<Qt::BrushStyle> outsideFillStyle = qlua::getBrush(L, 9);
  if (!outsideFillStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  const QBrush fill(outsideColor, *outsideFillStyle);
  return returnCode(L, getApi(L).TextRectangle(margins, offset, borderColor, borderWidth, fill));
}

static int L_WindowCircleOp(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const lua_Integer action = qlua::getInt(L, 2);
  const QRectF rect(
      qlua::getNumber(L, 3),
      qlua::getNumber(L, 4),
      qlua::getNumber(L, 5),
      qlua::getNumber(L, 6));
  const optional<QPen> pen = qlua::getPen(L, 7, 8, 9);
  const QColor brushColor = qlua::getQColor(L, 10);
  const optional<Qt::BrushStyle> brushStyle = qlua::getBrush(L, 11);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  if (!brushStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  const QBrush brush(brushColor, *brushStyle);

  switch (action)
  {
  case 1:
    return returnCode(L, getApi(L).WindowEllipse(windowName, rect, *pen, brush));
  case 2:
    return returnCode(L, getApi(L).WindowRect(windowName, rect, *pen, brush));
  case 3:
    return returnCode(
        L,
        getApi(L).WindowRoundedRect(
            windowName,
            rect,
            qlua::getNumber(L, 12),
            qlua::getNumber(L, 13),
            *pen,
            brush));
  case 4: // chord
  case 5: // pie
    return returnCode(L, ApiCode::OK);
  default:
    return returnCode(L, ApiCode::UnknownOption);
  }
}

static int L_WindowCreate(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location(qlua::getInt(L, 2), qlua::getInt(L, 3));
  const QSize size(qlua::getInt(L, 4), qlua::getInt(L, 5));
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 6);
  const MiniWindow::Flags flags = (MiniWindow::Flags)(int)qlua::getInt(L, 7);
  const QColor bg = qlua::getQColor(L, 8);
  if (!position) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(L, getApi(L).WindowCreate(windowName, location, size, *position, flags, bg));
}

static int L_WindowFont(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const string_view fontID = qlua::getString(L, 2);
  const QString fontName = qlua::getQString(L, 3);
  const qreal pointSize = qlua::getNumber(L, 4);
  if (pointSize == 0 && fontName.isEmpty()) [[unlikely]]
    return returnCode(L, getApi(L).WindowFontUnload(windowName, fontID));
  const bool bold = qlua::getBool(L, 5);
  const bool italic = qlua::getBool(L, 6);
  const bool underline = qlua::getBool(L, 7);
  const bool strikeout = qlua::getBool(L, 8);
  // const short charset = qlua::getInt(L, 9);
  const optional<QFont::StyleHint> hint = qlua::getFontHint(L, 10);
  if (!hint) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowFont(
          windowName,
          fontID,
          fontName,
          pointSize,
          bold,
          italic,
          underline,
          strikeout,
          *hint));
}

static int L_WindowGradient(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  QRectF rect(
      qlua::getNumber(L, 2),
      qlua::getNumber(L, 3),
      qlua::getNumber(L, 4),
      qlua::getNumber(L, 5));
  const QColor color1 = qlua::getQColor(L, 6);
  const QColor color2 = qlua::getQColor(L, 7);
  const lua_Integer mode = qlua::getInt(L, 7);
  if (mode != (lua_Integer)Qt::Horizontal && mode != (lua_Integer)Qt::Vertical) [[unlikely]]
    return returnCode(L, ApiCode::UnknownOption);
  return returnCode(
      L,
      getApi(L).WindowGradient(
          windowName,
          rect,
          color1,
          color2,
          (Qt::Orientation)mode));
}

static int L_WindowLine(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  QLineF line(
      qlua::getNumber(L, 2),
      qlua::getNumber(L, 3),
      qlua::getNumber(L, 4),
      qlua::getNumber(L, 5));
  const optional<QPen> pen = qlua::getPen(L, 6, 7, 8);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  return returnCode(L, getApi(L).WindowLine(windowName, line, *pen));
}

static int L_WindowPolygon(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const optional<QPolygonF> polygon = qlua::getQPolygonF(L, 2);
  const optional<QPen> pen = qlua::getPen(L, 3, 4, 5);
  const QColor brushColor = qlua::getQColor(L, 6);
  const optional<Qt::BrushStyle> brushStyle = qlua::getBrush(L, 7);
  const bool close = qlua::getBool(L, 8);
  const bool winding = qlua::getBool(L, 9);
  if (!polygon) [[unlikely]]
    return returnCode(L, ApiCode::InvalidNumberOfPoints);
  if (!pen) [[unlikely]]
    return returnCode(L, ApiCode::PenStyleNotValid);
  if (!brushStyle) [[unlikely]]
    return returnCode(L, ApiCode::BrushStyleNotValid);
  return returnCode(
      L,
      getApi(L).WindowPolygon(
          windowName,
          *polygon,
          *pen,
          QBrush(brushColor, *brushStyle),
          close,
          winding ? Qt::FillRule::WindingFill : Qt::FillRule::OddEvenFill));
}

static int L_WindowPosition(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const QPoint location(qlua::getInt(L, 2), qlua::getInt(L, 3));
  const optional<MiniWindow::Position> position = qlua::getWindowPosition(L, 4);
  const MiniWindow::Flags flags = (MiniWindow::Flags)(int)qlua::getInt(L, 5);
  if (!position) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(L, getApi(L).WindowPosition(windowName, location, *position, flags));
}

static int L_WindowRectOp(lua_State *L)
{
  const string_view windowName = qlua::getString(L, 1);
  const lua_Integer action = qlua::getInt(L, 2);
  const QRectF rect(
      qlua::getNumber(L, 3),
      qlua::getNumber(L, 4),
      qlua::getNumber(L, 5),
      qlua::getNumber(L, 6));
  switch (action)
  {
  case 1: // draw
    return returnCode(L, getApi(L).WindowRect(windowName, rect, qlua::getQColor(L, 7), QBrush()));
  case 2: // fill
    return returnCode(L, getApi(L).WindowRect(windowName, rect, QPen(), qlua::getQColor(L, 7)));
  case 3: // invert
    return returnCode(L, ApiCode::OK);
  case 4: // draw in two colors
    return returnCode(
        L,
        getApi(L).WindowFrame(windowName, rect, qlua::getQColor(L, 7), qlua::getQColor(L, 8)));
  case 5: // draw 3d edge
  case 6: // flood fill border
  case 7: // flood fill surface
    return returnCode(L, ApiCode::OK);
  default:
    return returnCode(L, ApiCode::UnknownOption);
  }
}

static int L_WindowResize(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowResize(
          qlua::getString(L, 1),
          QSize(qlua::getInt(L, 2), qlua::getInt(L, 3)),
          qlua::getQColor(L, 4)));
}

static int L_WindowSetZOrder(lua_State *L)
{
  return returnCode(L, getApi(L).WindowSetZOrder(qlua::getString(L, 1), qlua::getInt(L, 2)));
}

static int L_WindowShow(lua_State *L)
{
  return returnCode(L, getApi(L).WindowShow(qlua::getString(L, 1), qlua::getBool(L, 2)));
}

static int L_WindowText(lua_State *L)
{
  const qreal width = getApi(L).WindowText(
      qlua::getString(L, 1),
      qlua::getString(L, 2),
      qlua::getQString(L, 3),
      QRectF(
          qlua::getNumber(L, 4),
          qlua::getNumber(L, 5),
          qlua::getNumber(L, 6),
          qlua::getNumber(L, 7)),
      qlua::getQColor(L, 8));
  // qlua::getBool(L, 9) // unicode
  lua_pushinteger(L, width);
  return 1;
}

static int L_WindowTextWidth(lua_State *L)
{
  const int width = getApi(L).WindowTextWidth(
      qlua::getString(L, 1),
      qlua::getString(L, 2),
      qlua::getQString(L, 3));
  lua_pushinteger(L, width);
  return 1;
}

// window hotspots

static int L_WindowAddHotspot(lua_State *L)
{
  const string_view pluginID = qlua::getString(L, 1);
  const string_view windowName = qlua::getString(L, 2);
  const string_view hotspotID = qlua::getString(L, 3);
  const QRect geometry(
      QPoint(qlua::getInt(L, 4), qlua::getInt(L, 5)),
      QPoint(qlua::getInt(L, 6), qlua::getInt(L, 7)));
  Hotspot::Callbacks callbacks{
      .mouseOver = (string)qlua::getString(L, 8),
      .cancelMouseOver = (string)qlua::getString(L, 9),
      .mouseDown = (string)qlua::getString(L, 10),
      .cancelMouseDown = (string)qlua::getString(L, 11),
      .mouseUp = (string)qlua::getString(L, 12),
  };
  const QString &tooltip = qlua::getQString(L, 13);
  const optional<Qt::CursorShape> cursor = qlua::getCursor(L, 14);
  const bool trackHover = qlua::getInt(L, 15) & 0x01;
  if (!cursor) [[unlikely]]
    return returnCode(L, ApiCode::BadParameter);
  return returnCode(
      L,
      getApi(L).WindowAddHotspot(
          pluginID,
          windowName,
          hotspotID,
          geometry,
          std::move(callbacks),
          tooltip,
          *cursor,
          trackHover));
}

static int L_WindowDeleteHotspot(lua_State *L)
{
  return returnCode(L, getApi(L).WindowDeleteHotspot(qlua::getString(L, 1), qlua::getString(L, 2)));
}

static int L_WindowMoveHotspot(lua_State *L)
{
  return returnCode(
      L,
      getApi(L).WindowMoveHotspot(
          qlua::getString(L, 1),
          qlua::getString(L, 2),
          QRect(
              QPoint(qlua::getInt(L, 3), qlua::getInt(L, 4)),
              QPoint(qlua::getInt(L, 5), qlua::getInt(L, 6)))));
}

// userdata

static const struct luaL_Reg worldlib[] =
    // options
    {{"GetAlphaOption", L_GetAlphaOption},
     {"GetAlphaOptionList", L_GetAlphaOptionList},
     {"GetCurrentValue", L_GetCurrentValue},
     {"GetOption", L_GetOption},
     {"GetOptionList", L_GetOptionList},
     {"SetAlphaOption", L_SetAlphaOption},
     {"SetOption", L_SetOption},
     // output
     {"ColourNameToRGB", L_ColourNameToRGB},
     {"ColourNote", L_ColourNote},
     {"ColourTell", L_ColourTell},
     {"Hyperlink", L_Hyperlink},
     {"Note", L_Note},
     {"RGBColourToName", L_RGBColourToName},
     {"Send", L_Send},
     {"SendNoEcho", L_SendNoEcho},
     {"SetClipboard", L_SetClipboard},
     {"Tell", L_Tell},
     // plugins
     {"CallPlugin", L_CallPlugin},
     {"GetPluginInfo", L_GetPluginInfo},
     // senders
     {"EnableAlias", L_EnableAlias},
     {"EnableAliasGroup", L_EnableAliasGroup},
     {"EnablePlugin", L_EnablePlugin},
     {"EnableTimer", L_EnableTimer},
     {"EnableTimerGroup", L_EnableTimerGroup},
     {"EnableTrigger", L_EnableTrigger},
     {"EnableTriggerGroup", L_EnableTriggerGroup},
     {"IsAlias", L_IsAlias},
     {"IsTimer", L_IsTimer},
     {"IsTrigger", L_IsTrigger},
     // variables
     {"GetVariable", L_GetVariable},
     {"GetPluginVariable", L_GetPluginVariable},
     {"SetVariable", L_SetVariable},
     // windows
     {"Redraw", L_Redraw},
     {"Repaint", L_Repaint},
     {"TextRectangle", L_TextRectangle},
     {"WindowCircleOp", L_WindowCircleOp},
     {"WindowCreate", L_WindowCreate},
     {"WindowFont", L_WindowFont},
     {"WindowGradient", L_WindowGradient},
     {"WindowLine", L_WindowLine},
     {"WindowPolygon", L_WindowPolygon},
     {"WindowPosition", L_WindowPosition},
     {"WindowRectOp", L_WindowRectOp},
     {"WindowResize", L_WindowResize},
     {"WindowSetZOrder", L_WindowSetZOrder},
     {"WindowShow", L_WindowShow},
     {"WindowText", L_WindowText},
     {"WindowTextWidth", L_WindowTextWidth},
     // window hotspots
     {"WindowAddHotspot", L_WindowAddHotspot},
     {"WindowDeleteHotspot", L_WindowDeleteHotspot},
     {"WindowMoveHotspot", L_WindowMoveHotspot},

     {NULL, NULL}};

static int L_world_tostring(lua_State *L)
{
  lua_pushliteral(L, "world");
  return 1;
}

static const struct luaL_Reg worldlib_meta[] = {{"__tostring", L_world_tostring}, {NULL, NULL}};

void registerLuaWorld(lua_State *L)
{
  luaL_newlib(L, worldlib);

  luaL_newmetatable(L, WORLD_REG_KEY);
  luaL_setfuncs(L, worldlib_meta, 0);
  lua_setmetatable(L, -2);

  lua_setglobal(L, WORLD_LIB_KEY);

  lua_pushglobaltable(L);
  if (lua_getmetatable(L, -1) == LUA_TNIL)
    lua_newtable(L);

  lua_getglobal(L, WORLD_LIB_KEY);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  lua_pop(L, 1);
}
