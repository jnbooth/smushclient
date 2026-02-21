#pragma once
#include "../stringmap.h"
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

  enum class MergeMode : int64_t
  {
    Straight,
    Transparent,
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
                  qreal opacity,
                  const QRectF& sourceRect = QRectF());
  void clearHotspots();
  void deleteAllHotspots();
  bool deleteHotspot(std::string_view hotspotID);
  void drawArc(const QRectF& rect,
               const QPointF& start,
               const QPointF& end,
               const QPen& pen);
  void drawButton(const QRect& rect, ButtonFrame frame, ButtonFlags flags);
  void drawEllipse(const QRectF& rect,
                   const QPen& pen,
                   const QBrush& brush = QBrush());

  void drawEllipse(const QRectF& rect, const QBrush& brush)
  {
    drawEllipse(rect, Qt::PenStyle::NoPen, brush);
  }
  void drawFrame(const QRectF& rect,
                 const QColor& color1,
                 const QColor& color2);
  void drawGradient(const QRectF& rect, const QGradient& gradient);
  void drawImage(const QPixmap& image,
                 const QRectF& rect,
                 const QRectF& sourceRect = QRectF(),
                 qreal opacity = 1,
                 DrawImageMode mode = DrawImageMode::Copy);
  void drawImage(const QPixmap& image,
                 const QRectF& rect,
                 qreal opacity,
                 DrawImageMode mode = DrawImageMode::Copy)
  {
    drawImage(image, rect, QRectF(), opacity, mode);
  }
  void drawImage(const QPixmap& image,
                 const QRectF& rect,
                 const QRectF& sourceRect,
                 DrawImageMode mode)
  {
    drawImage(image, rect, sourceRect, 1, mode);
  }
  void drawImage(const QPixmap& image, const QRectF& rect, DrawImageMode mode)
  {
    drawImage(image, rect, QRectF(), 1, mode);
  }
  void drawImage(const QPixmap& image,
                 const QTransform& transform,
                 qreal opacity = 1,
                 MergeMode mode = MergeMode::Straight);
  void drawImage(const QPixmap& image,
                 const QTransform& transform,
                 MergeMode mode)
  {
    drawImage(image, transform, 1, mode);
  }
  void drawLine(const QLineF& line, const QPen& pen);
  void drawPolygon(const QPolygonF& polygon,
                   const QPen& pen,
                   const QBrush& brush = QBrush(),
                   Qt::FillRule fillRule = Qt::FillRule::OddEvenFill);
  void drawPolygon(const QPolygonF& polygon,
                   const QBrush& brush = QBrush(),
                   Qt::FillRule fillRule = Qt::FillRule::OddEvenFill)
  {
    drawPolygon(polygon, Qt::PenStyle::NoPen, brush, fillRule);
  }
  void drawPolyline(const QPolygonF& polygon, const QPen& pen);
  void drawRect(const QRectF& rect,
                const QPen& pen,
                const QBrush& brush = QBrush());
  void drawRect(const QRectF& rect, const QBrush& brush)
  {
    drawRect(rect, Qt::PenStyle::NoPen, brush);
  }
  void drawRoundedRect(const QRectF& rect,
                       qreal xRadius,
                       qreal yRadius,
                       const QPen& pen,
                       const QBrush& brush = QBrush());
  void drawRoundedRect(const QRectF& rect,
                       qreal xRadius,
                       qreal yRadius,
                       const QBrush& brush)
  {
    drawRoundedRect(rect, xRadius, yRadius, Qt::PenStyle::NoPen, brush);
  }
  QRectF drawText(const QFont& font,
                  const QString& text,
                  const QRectF& rect,
                  const QColor& color);
  constexpr bool drawsUnderneath() const noexcept
  {
    return flags.testFlag(Flag::DrawUnderneath);
  }
  QVariant execMenu(const QPoint& location, std::string_view menuString);
  const QFont* findFont(std::string_view fontID) const;
  Hotspot* findHotspot(std::string_view hotspotID) const;
  const QPixmap* findImage(std::string_view imageID) const;
  std::vector<std::string_view> fontList() const;
  constexpr const std::string& getPluginId() const noexcept { return pluginID; }
  constexpr const QPixmap& getPixmap() const noexcept { return pixmap; }
  constexpr int64_t getZOrder() const noexcept { return zOrder; }
  std::vector<std::string_view> hotspotList() const;
  std::vector<std::string_view> imageList() const;
  QVariant info(int64_t infoType) const;
  void invert(const QRect& rect,
              QImage::InvertMode mode = QImage::InvertMode::InvertRgb);
  const QFont& loadFont(std::string_view fontID, const QFont& font);
  const QPixmap& loadImage(std::string_view imageID, QPixmap&& image);
  const QPixmap& loadImage(std::string_view imageID, const QPixmap& image);
  bool mergeImageAlpha(const QPixmap& image,
                       const QPixmap& mask,
                       const QRect& targetRect,
                       const QRect& sourceRect = QRect(),
                       qreal opacity = 1,
                       MergeMode mode = MergeMode::Straight);
  bool mergeImageAlpha(const QPixmap& image,
                       const QPixmap& mask,
                       const QRect& targetRect,
                       qreal opacity,
                       MergeMode mode = MergeMode::Straight)
  {
    return mergeImageAlpha(image, mask, targetRect, QRect(), opacity, mode);
  }
  bool mergeImageAlpha(const QPixmap& image,
                       const QPixmap& mask,
                       const QRect& targetRect,
                       MergeMode mode)
  {
    return mergeImageAlpha(image, mask, targetRect, QRect(), 1, mode);
  }
  bool mergeImageAlpha(const QPixmap& image,
                       const QPixmap& mask,
                       const QRect& targetRect,
                       const QRect& sourceRect,
                       MergeMode mode)
  {
    return mergeImageAlpha(image, mask, targetRect, sourceRect, 1, mode);
  }
  void reset();
  bool setPixel(const QPoint& location, const QColor& color);
  void setPosition(const QPoint& location,
                   Position position,
                   Flags flags) noexcept;
  void setSize(const QSize& size, const QColor& fill) noexcept;
  void setSize(const QSize& size) noexcept;
  void setZOrder(int64_t zOrder) noexcept;
  bool unloadFont(std::string_view fontID);
  bool unloadImage(std::string_view imageID);
  void updatePosition();

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
