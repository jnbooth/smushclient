#pragma once
#include "../lookup.h"
#include "hotspot.h"
#include <QtCore/QDateTime>
#include <QtGui/QPainter>

enum class BlendMode : int64_t;
class ImageFilter;
class Plugin;
class WorldTab;

class MiniWindow : public QWidget
{
  Q_OBJECT

public:
  enum class DrawImageMode : int64_t
  {
    // Copy without stretching to the destination position. The image is not
    // clipped, so only the Left and Top parameters are used - the full image is
    // copied to that position.
    Copy = 1,
    // Stretch or shrink the image appropriately to fit into the rectangle:
    // Left, Top, Right, Bottom.
    Stretch,
    // Copy without stretching to the position Left, Top. However this is a
    // transparent copy, where the pixel at the left,top corner (pixel position
    // 0,0) is considered the transparent colour. Any pixels that exactly match
    // that colour are not copied. WARNING - do not choose black or white as the
    // transparent colour as that throws out the calculations. Choose some other
    // colour (eg. purple) - you won't see that colour anyway.
    CopyTransparent,
  };

  enum Flag
  {
    // Draw underneath. If set, the miniwindow is drawn beneath the scrolling
    // text in the output window.
    //
    // WARNING: If you set the "draw underneath" flag then you cannot use
    // hotspots, as the hotspots are underneath the text and will not be
    // detected.
    DrawUnderneath = 1,
    // Absolute location. If set, the miniwindow is not subject to auto
    // positioning (so the Position argument is ignored), and it is located
    // exactly at the Left, Top position designated in the function call. By
    // setting this bit you have absolute control over where the window will
    // appear.
    Absolute = 2,
    // Transparent. If set, whenever a pixel in the contents of the window
    // matches the BackgroundColour, it is not drawn, and the text underneath
    // shows through. This lets you make odd-shape windows like stars or
    // circles, by filling the outside (the part you don't want to see) with the
    // background colour. Ignore mouse. If set, this miniwindow is not
    // considered for mouse-over, mouse-down, mouse-up events.
    Transparent = 4,
    // Ignore mouse. If set, this miniwindow is not considered for mouse-over,
    // mouse-down, mouse-up events.
    IgnoreMouse = 8,
    // Keep existing hotspots. If set, hotspots are not deleted if you are
    // recreating an existing miniwindow.
    KeepHotspots = 16,
  };
  Q_DECLARE_FLAGS(Flags, Flag)

  enum class ButtonFrame : int64_t
  {
    Raised = 5,
    Etched = 6,
    Bump = 9,
    Sunken = 10,
  };

  enum ButtonFlag
  {
    Fill = 0x0800,
    Soft = 0x1000,
    Flat = 0x4000,
    Monochrome = 0x8000,
  };
  Q_DECLARE_FLAGS(ButtonFlags, ButtonFlag)

  enum class Position : int64_t
  {
    OutputStretch, // Stretch to output view size
    OutputScale,   // Scale to output view with aspect ratio
    OwnerStretch,  // Stretch to owner size
    OwnerScale,    // Scale to owner size with aspect ratio
    TopLeft,
    TopCenter,
    TopRight,
    CenterRight,
    BottomRight,
    BottomCenter,
    BottomLeft,
    CenterLeft,
    Center,
    Tile,
  };

  MiniWindow(const QPoint& location,
             const QSize& size,
             Position position,
             Flags flags,
             const QColor& fill,
             std::string_view pluginID,
             QWidget* parent = nullptr);
  Hotspot* addHotspot(std::string_view hotspotID,
                      WorldTab& tab,
                      const Plugin& plugin,
                      Hotspot::Callbacks&& callbacks);
  void applyFilter(const ImageFilter& filter, const QRect& rect = QRect());
  void blendImage(BlendMode mode,
                  const QPixmap& image,
                  const QRectF& rect,
                  qreal opacity = 1,
                  const QRectF& sourceRect = QRectF());
  void clearHotspots();
  bool deleteHotspot(std::string_view hotspotID);
  void drawButton(const QRect& rect, ButtonFrame frame, ButtonFlags flags);
  void drawEllipse(const QRectF& rect,
                   const QPen& pen,
                   const QBrush& brush = QBrush());
  void drawFrame(const QRectF& rect,
                 const QColor& color1,
                 const QColor& color2);
  void drawGradient(const QRectF& rect, const QGradient& gradient);
  void drawImage(const QPixmap& image,
                 const QRectF& rect,
                 const QRectF& sourceRect = QRectF(),
                 DrawImageMode = DrawImageMode::Copy,
                 qreal opacity = 1);
  void drawLine(const QLineF& line, const QPen& pen);
  void drawPolygon(const QPolygonF& polygon,
                   const QPen& pen,
                   const QBrush& brush,
                   Qt::FillRule fillRule);
  void drawPolyline(const QPolygonF& polygon, const QPen& pen);
  void drawRect(const QRectF& rect,
                const QPen& pen,
                const QBrush& brush = QBrush());
  void drawRoundedRect(const QRectF& rect,
                       qreal xRadius,
                       qreal yRadius,
                       const QPen& pen,
                       const QBrush& brush = QBrush());
  QRectF drawText(const QFont& font,
                  const QString& text,
                  const QRectF& rect,
                  const QColor& color);
  constexpr bool drawsUnderneath() const noexcept
  {
    return flags.testFlag(Flag::DrawUnderneath);
  }
  QVariant execMenu(const QPoint& location, std::string_view menuString);
  Hotspot* findHotspot(std::string_view hotspotID) const;
  constexpr const std::string& getPluginId() const noexcept { return pluginID; }
  constexpr const QPixmap& getPixmap() const noexcept { return pixmap; }
  constexpr int64_t getZOrder() const noexcept { return zOrder; }
  QVariant info(int64_t infoType) const;
  void invert(const QRect& rect,
              QImage::InvertMode mode = QImage::InvertMode::InvertRgb);
  void reset();
  void setPosition(const QPoint& location,
                   Position position,
                   Flags flags) noexcept;
  void setSize(const QSize& size, const QColor& fill) noexcept;
  void setZOrder(int64_t zOrder) noexcept;
  void updatePosition();

  const QFont* findFont(std::string_view fontID) const;
  const QFont& loadFont(std::string_view fontID, const QFont& font)
  {
    return fonts[std::string(fontID)] = font;
  }
  void unloadFont(std::string_view fontID) { fonts.erase(std::string(fontID)); }

  const QPixmap* findImage(std::string_view imageID) const;
  const QPixmap& loadImage(std::string_view imageID, const QPixmap&& image)
  {
    return images[std::string(imageID)] = image;
  }
  const QPixmap& loadImage(std::string_view imageID, const QPixmap& image)
  {
    return images[std::string(imageID)] = image;
  }
  void unloadImage(std::string_view imageID)
  {
    images.erase(std::string(imageID));
  }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  void applyFlags();
  QRect normalize(const QRect& rect) const noexcept;
  QRectF normalize(const QRectF& rect) const noexcept;
  void updateMask();

private:
  QColor background;
  QSize dimensions;
  QFlags<Flag> flags;
  string_map<QFont> fonts;
  string_map<std::unique_ptr<Hotspot>> hotspots;
  string_map<QPixmap> images;
  QDateTime installed;
  QPoint location;
  QPixmap pixmap;
  std::string pluginID;
  Position position;
  int64_t zOrder = 0;

private:
  class Painter : public QPainter
  {
  public:
    explicit Painter(MiniWindow* window);
    Painter(MiniWindow* window, QPainter::CompositionMode mode);
    Painter(MiniWindow* window, const QPen& pen);
    Painter(MiniWindow* window, const QPen& pen, const QBrush& brush);
    ~Painter();

  private:
    MiniWindow* window;
  };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MiniWindow::ButtonFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(MiniWindow::Flags)
