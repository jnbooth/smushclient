#pragma once
#include <string>
#include <QtGui/QEnterEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QWidget>

class MiniWindow;
class Plugin;

class Hotspot : public QWidget
{
  Q_OBJECT

public:
  enum struct EventFlag
  {
    Shift = 0x01,        // Shift key down
    Control = 0x02,      // Control key down
    Alt = 0x04,          // Alt key down
    MouseLeft = 0x10,    // LH mouse
    MouseRight = 0x20,   // RH mouse
    DoubleClick = 0x40,  // Double-click
    Hover = 0x80,        // Mouse-over not first time
    Scroll = 0x100,      // Scroll wheel scrolled (towards you)
    MouseMiddle = 0x200, // Middle mouse
  };
  Q_DECLARE_FLAGS(EventFlags, EventFlag)
  Q_FLAG(EventFlag)

  struct Callbacks
  {
    std::string mouseOver;
    std::string cancelMouseOver;
    std::string mouseDown;
    std::string cancelMouseDown;
    std::string mouseUp;
  };

  Hotspot(MiniWindow *parent, const Plugin *plugin, std::string_view id, Callbacks &&callbacks);
  void setCallbacks(Callbacks &&callbacks);
  inline bool belongsToPlugin(const Plugin *plugin) const noexcept { return plugin == this->plugin; }

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  Callbacks callbacks;
  bool hadMouseDown;
  std::string id;
  const Plugin *plugin;

  void runCallback(const std::string &callback, QEvent *event, EventFlags flags);
};
