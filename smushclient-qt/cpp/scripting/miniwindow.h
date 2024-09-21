#pragma once
#include <string>
#include <unordered_map>
#include <QtWidgets/QLabel>
#include <QtCore/QLine>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPolygon>
#include "hotspot.h"
#include "luaconf.h"

typedef LUA_INTEGER lua_Integer;
class Hotspot;
class Plugin;

class MiniWindow : public QWidget
{
public:
  enum struct DrawImageMode
  {
    // Copy without stretching to the destination position. The image is not clipped, so only the Left and Top parameters are used - the full image is copied to that position.
    Copy = 1,
    // Stretch or shrink the image appropriately to fit into the rectangle: Left, Top, Right, Bottom.
    Stretch = 2,
    // Copy without stretching to the position Left, Top. However this is a transparent copy, where the pixel at the left,top corner (pixel position 0,0) is considered the transparent colour. Any pixels that exactly match that colour are not copied. WARNING - do not choose black or white as the transparent colour as that throws out the calculations. Choose some other colour (eg. purple) - you won't see that colour anyway.
    CopyTransparent = 3,
  };

  enum Flag
  {
    // Draw underneath. If set, the miniwindow is drawn beneath the scrolling text in the output window.
    //
    // WARNING: If you set the "draw underneath" flag then you cannot use hotspots, as the hotspots are underneath the text and will not be detected.
    DrawUnderneath = 0x1,
    // Absolute location. If set, the miniwindow is not subject to auto positioning (so the Position argument is ignored), and it is located exactly at the Left, Top position designated in the function call. By setting this bit you have absolute control over where the window will appear.
    Absolute = 0x2,
    // Transparent. If set, whenever a pixel in the contents of the window matches the BackgroundColour, it is not drawn, and the text underneath shows through. This lets you make odd-shape windows like stars or circles, by filling the outside (the part you don't want to see) with the background colour.
    // Ignore mouse. If set, this miniwindow is not considered for mouse-over, mouse-down, mouse-up events.
    Transparent = 0x4,
    // Ignore mouse. If set, this miniwindow is not considered for mouse-over, mouse-down, mouse-up events.
    IgnoreMouse = 0x8,
    // Keep existing hotspots. If set, hotspots are not deleted if you are recreating an existing miniwindow.
    KeepHotspots = 0x10,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  enum struct Position : lua_Integer
  {
    OutputStretch = 0, // Stretch to output view size
    OutputScale = 1,   // Scale to output view with aspect ratio
    OwnerStretch = 2,  // Stretch to owner size
    OwnerScale = 3,    // Scale to owner size with aspect ratio
    TopLeft = 4,
    TopCenter = 5,
    TopRight = 6,
    CenterRight = 7,
    BottomRight = 8,
    BottomCenter = 9,
    BottomLeft = 10,
    CenterLeft = 11,
    Center = 12,
    Tile = 13,
  };

  MiniWindow(
      QWidget *parent,
      const QPoint &location,
      const QSize &size,
      Position position,
      Flags flags,
      const QColor &fill);
  Hotspot *addHotspot(
      std::string_view hotspotID,
      const Plugin *plugin,
      Hotspot::Callbacks &&callbacks);
  void clearHotspots();
  bool deleteHotspot(std::string_view hotspotID);
  void drawLine(const QLineF &line, const QPen &pen);
  void drawEllipse(const QRectF &rect, const QPen &pen, const QBrush &brush = QBrush());
  void drawFrame(const QRectF &rect, const QColor &color1, const QColor &color2);
  void drawGradient(const QRectF &rect, const QGradient &gradient);
  void drawImage(
      const QPixmap &image,
      const QRectF &rect,
      const QRectF &sourceRect = QRectF(),
      DrawImageMode = DrawImageMode::Copy,
      qreal opacity = 1);
  void drawPolygon(
      const QPolygonF &polygon,
      const QPen &pen,
      const QBrush &brush,
      Qt::FillRule fillRule);
  void drawPolyline(const QPolygonF &polygon, const QPen &pen);
  void drawRect(const QRectF &rect, const QPen &pen, const QBrush &brush = QBrush());
  void drawRoundedRect(
      const QRectF &rect,
      qreal xRadius,
      qreal yRadius,
      const QPen &pen,
      const QBrush &brush = QBrush());
  QRectF drawText(
      const QFont &font,
      const QString &text,
      const QRectF &rect,
      const QColor &color);
  inline bool drawsUnderneath() const noexcept { return flags.testFlag(Flag::DrawUnderneath); }
  Hotspot *findHotspot(std::string_view hotspotID) const;
  int getZOrder() const noexcept;
  inline const QPixmap &getPixmap() const noexcept { return pixmap; }
  void reset();
  void setPosition(const QPoint &location, Position position, Flags flags) noexcept;
  void setSize(const QSize &size, const QColor &fill) noexcept;
  void setZOrder(int zOrder) noexcept;
  void updatePosition();

  const QFont *getFont(std::string_view fontID) const;
  inline const QFont &loadFont(std::string_view fontID, const QFont &font)
  {
    return fonts[(std::string)fontID] = font;
  }
  inline void unloadFont(std::string_view fontID)
  {
    fonts.erase((std::string)fontID);
  }

  const QPixmap *getImage(std::string_view imageID) const;
  inline const QPixmap &loadImage(std::string_view imageID, const QPixmap &&image)
  {
    return images[(std::string)imageID] = image;
  }
  inline const QPixmap &loadImage(std::string_view imageID, const QPixmap &image)
  {
    return images[(std::string)imageID] = image;
  }
  inline void unloadImage(std::string_view imageID)
  {
    images.erase((std::string)imageID);
  }

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QColor background;
  QSize dimensions;
  QFlags<Flag> flags;
  std::unordered_map<std::string, QFont> fonts;
  std::unordered_map<std::string, Hotspot *> hotspots;
  std::unordered_map<std::string, QPixmap> images;
  QPoint location;
  QPixmap pixmap;
  Position position;
  int zOrder;

  void applyFlags();
  void updateMask();

  class Painter : public QPainter
  {
  public:
    explicit Painter(MiniWindow *window);
    Painter(MiniWindow *window, const QPen &pen);
    Painter(MiniWindow *window, const QPen &pen, const QBrush &brush);
    ~Painter();

  private:
    MiniWindow *window;
  };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MiniWindow::Flags)
