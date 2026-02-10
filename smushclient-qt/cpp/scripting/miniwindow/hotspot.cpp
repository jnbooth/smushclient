#include "hotspot.h"
#include "../callback/plugincallback.h"
#include "../plugin.h"
#include "../qlua.h"
#include "../ui/worldtab.h"
#include "miniwindow.h"
#include <QtGui/QGuiApplication>
extern "C"
{
#include "lua.h"
}

using std::string;
using std::string_view;

// Private utils

namespace {
constexpr bool
hasCallback(const std::string& callback)
{
  return !callback.empty();
}

inline bool
hasCallback(const std::string& callback, QEvent* event)
{
  const bool willHandle = hasCallback(callback);
  event->setAccepted(willHandle);
  return willHandle;
}

constexpr Hotspot::EventFlags
getEventFlags(Qt::KeyboardModifiers modifiers,
              Qt::MouseButtons buttons) noexcept
{
  Hotspot::EventFlags flags;
  if (modifiers.testFlag(Qt::KeyboardModifier::ShiftModifier)) {
    flags.setFlag(Hotspot::EventFlag::Shift);
  }
  if (modifiers.testFlag(Qt::KeyboardModifier::ControlModifier)) {
    flags.setFlag(Hotspot::EventFlag::Control);
  }
  if (modifiers.testFlag(Qt::KeyboardModifier::AltModifier)) {
    flags.setFlag(Hotspot::EventFlag::Alt);
  }
  if (buttons.testFlag(Qt::MouseButton::LeftButton)) {
    flags.setFlag(Hotspot::EventFlag::MouseLeft);
  }
  if (buttons.testFlag(Qt::MouseButton::RightButton)) {
    flags.setFlag(Hotspot::EventFlag::MouseRight);
  }
  if (buttons.testFlag(Qt::MouseButton::MiddleButton)) {
    flags.setFlag(Hotspot::EventFlag::MouseMiddle);
  }
  return flags;
}

Hotspot::EventFlags
getEventFlags(const QSinglePointEvent* event)
{
  return getEventFlags(event->modifiers(), event->buttons() | event->button());
}

Hotspot::EventFlags
getEventFlags()
{
  return getEventFlags(QGuiApplication::keyboardModifiers(),
                       QGuiApplication::mouseButtons());
}
} // namespace

// Public methods

Hotspot::Hotspot(WorldTab& tab,
                 const Plugin& plugin,
                 string_view id,
                 Callbacks&& callbacksMoved,
                 QWidget* parent)
  : QWidget(parent)
  , callbacks(std::move(callbacksMoved))
  , id(id)
  , plugin(plugin)
  , tab(tab)
{
}

void
Hotspot::finishDrag()
{
  hadDrag = false;
  if (hasCallback(callbacks.dragRelease)) {
    runCallback(callbacks.dragRelease, getEventFlags());
  }
}

const Hotspot::Callbacks&
Hotspot::setCallbacks(Callbacks&& newCallbacks)
{
  hadMouseDown = false;
  callbacks = std::move(newCallbacks);
  return callbacks;
}

const Hotspot::Callbacks&
Hotspot::setCallbacks(CallbacksPartial&& partial)
{
  hadMouseDown = false;
  if (partial.dragMove) {
    callbacks.dragMove = std::move(*partial.dragMove);
  }
  if (partial.dragRelease) {
    callbacks.dragRelease = std::move(*partial.dragRelease);
  }
  if (partial.mouseOver) {
    callbacks.mouseOver = std::move(*partial.mouseOver);
  }
  if (partial.cancelMouseOver) {
    callbacks.cancelMouseOver = std::move(*partial.cancelMouseOver);
  }
  if (partial.mouseDown) {
    callbacks.mouseDown = std::move(*partial.mouseDown);
  }
  if (partial.cancelMouseDown) {
    callbacks.cancelMouseDown = std::move(*partial.cancelMouseDown);
  }
  if (partial.mouseUp) {
    callbacks.mouseUp = std::move(*partial.mouseUp);
  }
  if (partial.scroll) {
    callbacks.scroll = std::move(*partial.scroll);
  }
  return callbacks;
}

// Protected overrides

void
Hotspot::enterEvent(QEnterEvent* event)
{
  if (hasCallback(callbacks.mouseOver, event)) {
    runCallback(callbacks.mouseOver, getEventFlags(event));
  }
}

void
Hotspot::leaveEvent(QEvent* event)
{
  const bool hasCancelMouseOver = hasCallback(callbacks.cancelMouseOver);
  const bool hasCancelMouseDown =
    hadMouseDown && hasCallback(callbacks.cancelMouseDown);
  hadMouseDown = false;

  if (!hasCancelMouseOver && !hasCancelMouseDown) {
    event->ignore();
    return;
  }

  event->accept();
  const EventFlags flags = getEventFlags();

  if (hasCancelMouseOver) {
    runCallback(callbacks.cancelMouseOver, flags);
  }

  if (hasCancelMouseDown) {
    runCallback(callbacks.cancelMouseDown, flags);
  }
}

void
Hotspot::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (hasCallback(callbacks.mouseDown, event)) {
    runCallback(callbacks.mouseDown,
                getEventFlags(event) | EventFlag::DoubleClick);
  }
}

void
Hotspot::mouseMoveEvent(QMouseEvent* event)
{
  if (hadMouseDown && !hadDrag) {
    startDrag(event);
  }

  if (hasCallback(callbacks.mouseOver)) {
    runCallback(callbacks.mouseOver, getEventFlags(event) | EventFlag::Hover);
  }

  event->ignore();
}

void
Hotspot::mousePressEvent(QMouseEvent* event)
{
  hadMouseDown = true;
  if (hasCallback(callbacks.mouseDown, event)) {
    runCallback(callbacks.mouseDown, getEventFlags(event));
  }
}

void
Hotspot::mouseReleaseEvent(QMouseEvent* event)
{
  hadMouseDown = false;

  if (hasCallback(callbacks.mouseUp)) {
    runCallback(callbacks.mouseUp, getEventFlags(event));
  }

  event->ignore();
}

void
Hotspot::wheelEvent(QWheelEvent* event)
{
  if (!hasCallback(callbacks.scroll, event)) {
    return;
  }

  const int yDelta = event->angleDelta().y();
  if (yDelta > 0) {
    runCallback(callbacks.scroll, getEventFlags(event) | EventFlag::ScrollDown);
  } else if (yDelta < 0) {
    runCallback(callbacks.scroll, getEventFlags(event));
  }
}

// Private methods

class HotspotCallback : public DynamicPluginCallback
{
public:
  HotspotCallback(const string& callback,
                  Hotspot::EventFlags flags,
                  const string& hotspotID)
    : DynamicPluginCallback(callback)
    , flags(flags)
    , hotspotID(hotspotID)
  {
  }

  constexpr ActionSource source() const noexcept override
  {
    return ActionSource::Hotspot;
  }

  int pushArguments(lua_State* L) const override
  {
    lua_pushinteger(L, flags);
    qlua::pushString(L, hotspotID);
    return 2;
  }

private:
  Hotspot::EventFlags flags;
  const string& hotspotID;
};

void
Hotspot::runCallback(const string& callbackName, EventFlags flags)
{
  HotspotCallback callback(callbackName, flags, id);
  plugin.runCallback(callback);
}

void
Hotspot::startDrag(QMouseEvent* event)
{
  hadDrag = true;
  tab.setOnDragRelease(this);

  if (!hasCallback(callbacks.dragMove)) {
    return;
  }

  HotspotCallback callback(callbacks.dragMove, getEventFlags(event), id);
  lua_State* L = plugin.state();

  if (!callback.findCallback(L)) {
    return;
  }

  const int nargs = callback.pushArguments(L);

  tab.setOnDragMove(CallbackTrigger(L, nargs, this));
}
