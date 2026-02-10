#pragma once
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
    Shift = 1,         // Shift key down
    Control = 2,       // Control key down
    Alt = 4,           // Alt key down
    MouseLeft = 16,    // LH mouse
    MouseRight = 32,   // RH mouse
    DoubleClick = 64,  // Double-click
    Hover = 128,       // Mouse-over not first time
    ScrollDown = 256,  // Scroll wheel scrolled (towards you)
    MouseMiddle = 512, // Middle mouse
  };
  Q_DECLARE_FLAGS(EventFlags, EventFlag)

  enum Flag
  {
    ReportAllMouseovers = 0x01,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  template<typename T>
  struct BasicCallbacks
  {
    T dragMove{};
    T dragRelease{};
    T mouseOver{};
    T cancelMouseOver{};
    T mouseDown{};
    T cancelMouseDown{};
    T mouseUp{};
    T scroll{};
  };
  using Callbacks = BasicCallbacks<std::string>;
  using CallbacksPartial = BasicCallbacks<std::optional<std::string>>;

  Hotspot(WorldTab* tab,
          const Plugin* plugin,
          std::string_view id,
          Callbacks&& callbacks,
          QWidget* parent = nullptr);
  constexpr bool belongsToPlugin(const Plugin* otherPlugin) const noexcept
  {
    return otherPlugin == plugin;
  }
  void finishDrag();
  QVariant info(int64_t infoType) const;
  const Callbacks& setCallbacks(Callbacks&& callbacks);
  const Callbacks& setCallbacks(CallbacksPartial&& partial);

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  void runCallback(const std::string& callback, EventFlags flags);
  void startDrag(QMouseEvent* event);

private:
  Callbacks callbacks;
  bool hadDrag = false;
  bool hadMouseDown = false;
  std::string id;
  const Plugin* plugin;
  WorldTab* tab;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Hotspot::EventFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hotspot::Flags)
