#pragma once
#include <optional>
#include <string>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QEnterEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QWidget>

class MiniWindow;
class Plugin;
class WorldTab;

class Hotspot : public QWidget
{
public:
  enum EventFlag
  {
    Shift = 0x01,        // Shift key down
    Control = 0x02,      // Control key down
    Alt = 0x04,          // Alt key down
    MouseLeft = 0x10,    // LH mouse
    MouseRight = 0x20,   // RH mouse
    DoubleClick = 0x40,  // Double-click
    Hover = 0x80,        // Mouse-over not first time
    ScrollDown = 0x100,  // Scroll wheel scrolled (towards you)
    MouseMiddle = 0x200, // Middle mouse
  };
  Q_DECLARE_FLAGS(EventFlags, EventFlag)

  enum Flag
  {
    ReportAllMouseovers = 0x01,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  template <typename T>
  struct BasicCallbacks
  {
    T dragMove;
    T dragRelease;
    T mouseOver;
    T cancelMouseOver;
    T mouseDown;
    T cancelMouseDown;
    T mouseUp;
    T scroll;
  };
  using Callbacks = BasicCallbacks<std::string>;
  using CallbacksPartial = BasicCallbacks<std::optional<std::string>>;

  Hotspot(MiniWindow *parent, WorldTab *tab, const Plugin *plugin, std::string_view id, Callbacks &&callbacks);
  const Callbacks &setCallbacks(Callbacks &&callbacks);
  const Callbacks &setCallbacks(CallbacksPartial &&callbacks);
  inline bool belongsToPlugin(const Plugin *otherPlugin) const noexcept
  {
    return otherPlugin == plugin;
  }
  void finishDrag();
  QVariant info(int infoType) const;

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  Callbacks callbacks;
  bool hadDrag = false;
  bool hadMouseDown = false;
  std::string id;
  const Plugin *plugin;
  WorldTab *tab;

  void runCallback(const std::string &callback, EventFlags flags);
  void startDrag(QMouseEvent *event);
};
