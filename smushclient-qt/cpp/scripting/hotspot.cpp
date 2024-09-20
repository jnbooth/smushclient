#include "hotspot.h"
#include <QtGui/QGuiApplication>
#include "miniwindow.h"
#include "plugin.h"

using std::string;
using std::string_view;

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
  return getEventFlags(event->modifiers(), event->buttons());
}

Hotspot::EventFlags getEventFlags()
{
  return getEventFlags(QGuiApplication::keyboardModifiers(), QGuiApplication::mouseButtons());
}

Hotspot::Hotspot(MiniWindow *parent, const Plugin *plugin, string_view id, Callbacks &&callbacks)
    : QWidget(parent),
      callbacks(std::move(callbacks)),
      id(id),
      plugin(plugin)
{
  setWindowOpacity(0);
}

void Hotspot::setCallbacks(Callbacks &&newCallbacks)
{
  hadMouseDown = false;
  callbacks = std::move(newCallbacks);
}

// Protected overrides

void Hotspot::enterEvent(QEnterEvent *event)
{
  runCallback(callbacks.mouseOver, event, getEventFlags(event));
}

void Hotspot::leaveEvent(QEvent *event)
{
  const bool handlesMouseDown = hadMouseDown && !callbacks.cancelMouseDown.empty();
  const bool handlesMouseOver = !callbacks.cancelMouseOver.empty();
  hadMouseDown = false;

  if (!handlesMouseDown && !handlesMouseOver)
  {
    event->ignore();
    return;
  }
  event->accept();

  lua_Integer flags = (lua_Integer)getEventFlags();

  if (handlesMouseDown)
    plugin->runCallback(callbacks.cancelMouseDown, id, flags);

  if (handlesMouseOver)
    plugin->runCallback(callbacks.cancelMouseOver, id, flags);
}

void Hotspot::mouseDoubleClickEvent(QMouseEvent *event)
{
  runCallback(callbacks.mouseDown, event, getEventFlags(event) | EventFlag::DoubleClick);
}

void Hotspot::mouseMoveEvent(QMouseEvent *event)
{
  if (!hasMouseTracking())
  {
    event->ignore();
    return;
  }
  runCallback(callbacks.mouseOver, event, getEventFlags(event) | EventFlag::Hover);
}

void Hotspot::mousePressEvent(QMouseEvent *event)
{
  hadMouseDown = true;
  runCallback(callbacks.mouseDown, event, getEventFlags(event));
}

void Hotspot::mouseReleaseEvent(QMouseEvent *event)
{
  hadMouseDown = false;
  runCallback(callbacks.mouseUp, event, getEventFlags(event));
}

void Hotspot::runCallback(const string &callback, QEvent *event, EventFlags flags)
{
  if (callback.empty())
  {
    event->ignore();
    return;
  }
  event->accept();
  plugin->runCallback(callback, id, (lua_Integer)flags);
}
