#include "hotspot.h"
#include <QtGui/QGuiApplication>
#include "miniwindow.h"
#include "plugin.h"
#include "plugincallback.h"
#include "qlua.h"
#include "../ui/worldtab.h"
extern "C"
{
#include "lua.h"
}

using std::optional;
using std::string;
using std::string_view;

// Private utils

inline constexpr const char *getProperty(const string &callback)
{
  const size_t n = callback.find((char)0);
  return n == string::npos ? nullptr : callback.data() + n + 1;
}

constexpr bool hasCallback(const std::string &callback)
{
  return !callback.empty();
}

inline bool hasCallback(const std::string &callback, QEvent *event)
{
  const bool willHandle = hasCallback(callback);
  event->setAccepted(willHandle);
  return willHandle;
}

constexpr void replaceFirstStop(string &callback) noexcept
{
  const size_t n = callback.find('.');
  if (n != string::npos)
    callback[n] = 0;
}

constexpr void replaceFirstStops(Hotspot::Callbacks &callbacks) noexcept
{
  replaceFirstStop(callbacks.cancelMouseDown);
  replaceFirstStop(callbacks.cancelMouseOver);
  replaceFirstStop(callbacks.dragMove);
  replaceFirstStop(callbacks.dragRelease);
  replaceFirstStop(callbacks.mouseDown);
  replaceFirstStop(callbacks.mouseOver);
  replaceFirstStop(callbacks.mouseUp);
  replaceFirstStop(callbacks.scroll);
}

// Public methods

constexpr Hotspot::EventFlags getEventFlags(Qt::KeyboardModifiers modifiers, Qt::MouseButtons buttons) noexcept
{
  Hotspot::EventFlags flags;
  if (modifiers.testFlag(Qt::KeyboardModifier::ShiftModifier))
    flags.setFlag(Hotspot::EventFlag::Shift);
  if (modifiers.testFlag(Qt::KeyboardModifier::ControlModifier))
    flags.setFlag(Hotspot::EventFlag::Control);
  if (modifiers.testFlag(Qt::KeyboardModifier::AltModifier))
    flags.setFlag(Hotspot::EventFlag::Alt);
  if (buttons.testFlag(Qt::MouseButton::LeftButton))
    flags.setFlag(Hotspot::EventFlag::MouseLeft);
  if (buttons.testFlag(Qt::MouseButton::RightButton))
    flags.setFlag(Hotspot::EventFlag::MouseRight);
  if (buttons.testFlag(Qt::MouseButton::MiddleButton))
    flags.setFlag(Hotspot::EventFlag::MouseMiddle);
  return flags;
}

Hotspot::EventFlags getEventFlags(const QSinglePointEvent *event)
{
  return getEventFlags(event->modifiers(), event->buttons() | event->button());
}

Hotspot::EventFlags getEventFlags()
{
  return getEventFlags(QGuiApplication::keyboardModifiers(), QGuiApplication::mouseButtons());
}

Hotspot::Hotspot(
    MiniWindow *parent,
    WorldTab *tab,
    const Plugin *plugin,
    string_view id,
    Callbacks &&callbacksMoved)
    : QWidget(parent),
      callbacks(std::move(callbacksMoved)),
      hadDrag(false),
      hadMouseDown(false),
      id(id),
      plugin(plugin),
      tab(tab)
{
  replaceFirstStops(callbacks);
}

const Hotspot::Callbacks &Hotspot::setCallbacks(Callbacks &&newCallbacks)
{
  hadMouseDown = false;
  callbacks = std::move(newCallbacks);
  replaceFirstStops(callbacks);
  return callbacks;
}

const Hotspot::Callbacks &Hotspot::setCallbacks(CallbacksPartial &&partial)
{
  hadMouseDown = false;
  if (partial.dragMove)
    callbacks.dragMove = std::move(*partial.dragMove);
  if (partial.dragRelease)
    callbacks.dragRelease = std::move(*partial.dragRelease);
  if (partial.mouseOver)
    callbacks.mouseOver = std::move(*partial.mouseOver);
  if (partial.cancelMouseOver)
    callbacks.cancelMouseOver = std::move(*partial.cancelMouseOver);
  if (partial.mouseDown)
    callbacks.mouseDown = std::move(*partial.mouseDown);
  if (partial.cancelMouseDown)
    callbacks.cancelMouseDown = std::move(*partial.cancelMouseDown);
  if (partial.mouseUp)
    callbacks.mouseUp = std::move(*partial.mouseUp);
  if (partial.scroll)
    callbacks.scroll = std::move(*partial.scroll);
  replaceFirstStops(callbacks);
  return callbacks;
}

void Hotspot::finishDrag()
{
  hadDrag = false;
  runCallback(callbacks.dragRelease, getEventFlags());
}

// Protected overrides

void Hotspot::enterEvent(QEnterEvent *event)
{
  if (hasCallback(callbacks.mouseOver, event))
    runCallback(callbacks.mouseOver, getEventFlags(event));
}

void Hotspot::leaveEvent(QEvent *event)
{
  const bool hasCancelMouseOver = hasCallback(callbacks.cancelMouseOver);
  const bool hasCancelMouseDown = hadMouseDown && hasCallback(callbacks.cancelMouseDown);
  hadMouseDown = false;

  if (!hasCancelMouseOver && !hasCancelMouseDown)
  {
    event->ignore();
    return;
  }

  event->accept();
  const EventFlags flags = getEventFlags();

  if (hasCancelMouseOver)
    runCallback(callbacks.cancelMouseOver, flags);

  if (hasCancelMouseDown)
    runCallback(callbacks.cancelMouseDown, flags);
}

void Hotspot::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (hasCallback(callbacks.mouseDown, event))
    runCallback(callbacks.mouseDown, getEventFlags(event) | EventFlag::DoubleClick);
}

void Hotspot::mouseMoveEvent(QMouseEvent *event)
{
  if (hadMouseDown && !hadDrag)
    startDrag(event);

  if (hasCallback(callbacks.mouseOver))
    runCallback(callbacks.mouseOver, getEventFlags(event) | EventFlag::Hover);

  event->ignore();
}

void Hotspot::mousePressEvent(QMouseEvent *event)
{
  hadMouseDown = true;
  if (hasCallback(callbacks.mouseDown, event))
    runCallback(callbacks.mouseDown, getEventFlags(event));
}

void Hotspot::mouseReleaseEvent(QMouseEvent *event)
{
  hadMouseDown = false;
  if (hasCallback(callbacks.mouseUp))
    runCallback(callbacks.mouseUp, getEventFlags(event));
}

void Hotspot::wheelEvent(QWheelEvent *event)
{
  if (!hasCallback(callbacks.scroll, event))
    return;
  const int yDelta = event->angleDelta().y();
  if (yDelta > 0)
    runCallback(callbacks.scroll, getEventFlags(event) | EventFlag::ScrollDown);
  else if (yDelta < 0)
    runCallback(callbacks.scroll, getEventFlags(event));
}

// Private methods

class HotspotCallback : public PluginCallback
{
public:
  HotspotCallback(const string &callback, Hotspot::EventFlags flags, const string &hotspotID)
      : callback(callback.data()),
        flags(flags),
        hotspotID(hotspotID),
        prop(getProperty(callback)) {}

  inline constexpr const char *name() const noexcept override { return callback; }
  inline constexpr const char *property() const noexcept override { return prop; }
  inline constexpr ActionSource source() const noexcept override { return ActionSource::Hotspot; }

  int pushArguments(lua_State *L) const override
  {
    lua_pushinteger(L, flags);
    qlua::pushString(L, hotspotID);
    return 2;
  }

private:
  const char *callback;
  Hotspot::EventFlags flags;
  const string &hotspotID;
  const char *prop;
};

inline void Hotspot::runCallback(const string &callbackName, EventFlags flags)
{
  HotspotCallback callback(callbackName, flags, id);
  plugin->runCallback(callback);
}

void Hotspot::startDrag(QMouseEvent *event)
{
  hadDrag = true;
  if (hasCallback(callbacks.dragRelease))
    tab->setOnDragRelease(this);

  if (!hasCallback(callbacks.dragMove))
    return;

  HotspotCallback callback(callbacks.dragMove, getEventFlags(event), id);
  lua_State *L = plugin->state();

  if (!callback.findCallback(L))
    return;

  const int nargs = callback.pushArguments(L);

  tab->setOnDragMove(CallbackTrigger(L, nargs));
}
