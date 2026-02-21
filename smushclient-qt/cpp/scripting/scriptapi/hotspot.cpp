#include "../scriptapi.h"

using std::string;
using std::string_view;
using std::vector;

#define TRY_WINDOW(windowName)                                                 \
  findWindow(windowName);                                                      \
  if (window == nullptr) [[unlikely]] {                                        \
    return ApiCode::NoSuchWindow;                                              \
  }

#define TRY_PIXMAP(window, imageID)                                            \
  window->findImage(imageID);                                                  \
  if (pixmap == nullptr) [[unlikely]] {                                        \
    return ApiCode::ImageNotInstalled;                                         \
  }

#define TRY_HOTSPOT(window, hotspotID)                                         \
  window->findHotspot(hotspotID);                                              \
  if (hotspot == nullptr) [[unlikely]] {                                       \
    return ApiCode::HotspotNotInstalled;                                       \
  }

#define CHECK_NONNULL(ptr)                                                     \
  if ((ptr) == nullptr) [[unlikely]] {                                         \
    return QVariant();                                                         \
  }

// Public methods

ApiCode
ScriptApi::WindowAddHotspot(size_t index,
                            string_view windowName,
                            string_view hotspotID,
                            const QRect& geometry,
                            Hotspot::Callbacks&& callbacks,
                            const QString& tooltip,
                            Qt::CursorShape cursorShape,
                            Hotspot::Flags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot =
    window->addHotspot(hotspotID, tab, plugins[index], std::move(callbacks));
  if (hotspot == nullptr) [[unlikely]] {
    return ApiCode::HotspotPluginChanged;
  }
  hotspot->setGeometry(geometry);
  hotspot->setToolTip(tooltip);
  hotspot->setCursor(cursorShape);
  hotspot->setMouseTracking(flags.testFlag(Hotspot::Flag::ReportAllMouseovers));
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDeleteAllHotspots(string_view windowName) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->deleteAllHotspots();
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDeleteHotspot(string_view windowName,
                               string_view hotspotID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  if (!window->deleteHotspot(hotspotID)) [[unlikely]] {
    return ApiCode::HotspotNotInstalled;
  }
  return ApiCode::OK;
}

QVariant
ScriptApi::WindowHotspotInfo(string_view windowName,
                             string_view hotspotID,
                             int64_t infoType) const
{

  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  const Hotspot* hotspot = window->findHotspot(hotspotID);
  CHECK_NONNULL(hotspot);
  return hotspot->info(infoType);
}

vector<string_view>
ScriptApi::WindowHotspotList(std::string_view windowName) const
{
  MiniWindow* window = findWindow(windowName);
  return window == nullptr ? vector<string_view>() : window->hotspotList();
}

ApiCode
ScriptApi::WindowHotspotTooltip(string_view windowName,
                                string_view hotspotID,
                                const QString& tooltip) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot = TRY_HOTSPOT(window, hotspotID);
  hotspot->setToolTip(tooltip);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowMoveHotspot(string_view windowName,
                             string_view hotspotID,
                             const QRect& geometry) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot = TRY_HOTSPOT(window, hotspotID);
  hotspot->setGeometry(geometry);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowUpdateHotspot(size_t index,
                               string_view windowName,
                               string_view hotspotID,
                               Hotspot::CallbacksPartial&& callbacks) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot = TRY_HOTSPOT(window, hotspotID);
  if (!hotspot->belongsToPlugin(plugins[index])) {
    return ApiCode::HotspotPluginChanged;
  }
  hotspot->setCallbacks(std::move(callbacks));
  return ApiCode::OK;
}
