#include "../../image.h"
#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../miniwindow/imagefilters.h"
#include "../scriptapi.h"
#include <QtCore/QFile>
#include <QtGui/QFontDatabase>
#include <QtGui/QGradient>
#include <QtGui/QPainterPath>

using std::optional;
using std::string;
using std::string_view;

// Private utils

#define TRY_WINDOW(windowName)                                                 \
  findWindow((windowName));                                                    \
  if (window == nullptr) [[unlikely]] {                                        \
    return ApiCode::NoSuchWindow;                                              \
  }

#define TRY_PIXMAP(window, imageID)                                            \
  (window)->findImage((imageID));                                              \
  if (pixmap == nullptr) [[unlikely]] {                                        \
    return ApiCode::ImageNotInstalled;                                         \
  }

#define CHECK_NONNULL(ptr)                                                     \
  if ((ptr) == nullptr) [[unlikely]] {                                         \
    return {};                                                                 \
  }

// Public static methods

QColor
ScriptApi::BlendPixel(const QColor& blend,
                      const QColor& base,
                      BlendMode mode,
                      qreal opacity)
{
  QPixmap blendPixel = image::colorPixel(blend);
  QPixmap basePixel = image::colorPixel(base);
  image::blend(basePixel, blendPixel, {}, mode, opacity);
  return basePixel.toImage().pixelColor({ 0, 0 });
}

// Public methods

ApiCode
ScriptApi::WindowArc(string_view windowName,
                     const QRectF& rect,
                     const QPointF& start,
                     const QPointF& end,
                     const QPen& pen) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawArc(rect, start, end, pen);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowBezier(string_view windowName,
                        const QList<QPointF>& points,
                        const QPen& pen) const
{
  const qsizetype size = points.size();
  if (size < 4 || size % 3 != 1) {
    return ApiCode::InvalidNumberOfPoints;
  }
  QPainterPath path;
  auto iter = points.begin();
  path.moveTo(*iter);
  for (auto end = points.end(); iter != end;) {
    path.cubicTo(*++iter, *++iter, *++iter);
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawPath(path, pen);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowBlendImage(string_view windowName,
                            string_view imageID,
                            const QRectF& rect,
                            BlendMode mode,
                            qreal opacity,
                            const QRectF& sourceRect) const
{
  if (opacity < 0 || opacity > 1) [[unlikely]] {
    return ApiCode::BadParameter;
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  window->blendImage(mode, *pixmap, rect, opacity, sourceRect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowButton(string_view windowName,
                        const QRect& rect,
                        ButtonFrame frame,
                        MiniWindow::ButtonFlags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawButton(rect, frame, flags);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowCreate(size_t index,
                        string_view name,
                        const QPoint& location,
                        const QSize& size,
                        MiniWindow::Position position,
                        MiniWindow::Flags flags,
                        const QColor& fill)
{
  if (name.empty()) [[unlikely]] {
    return ApiCode::NoNameSpecified;
  }
  if (!size.isValid()) [[unlikely]] {
    return ApiCode::BadParameter;
  }

  std::unique_ptr<MiniWindow>& window = windows[string(name)];
  if (window == nullptr) {
    window = std::make_unique<MiniWindow>(
      location, size, position, flags, fill, plugins[index].id(), tab.ui->area);
  } else {
    window->setPosition(location, position, flags);
    window->setSize(size, fill);
    window->reset();
  }
  window->updatePosition();
  stackWindow(name, *window);
  window->hide();
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowCreateImage(string_view windowName,
                             string_view imageID,
                             std::array<int64_t, 8> rows) const
{
  constexpr const QSize grid(8, 8);
  static_assert(sizeof(rows) ==
                sizeof(unsigned char) * grid.height() * grid.width());

  MiniWindow* window = TRY_WINDOW(windowName);

  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: see static assertion above
  const unsigned char* data =
    reinterpret_cast<const unsigned char*>(rows.data());
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

  QBitmap bitmap = QBitmap::fromData(grid, data, image::bitmapFormat);
  window->loadImage(imageID, std::move(bitmap));
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDelete(string_view windowName) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  delete window;
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDrawImage(string_view windowName,
                           string_view imageID,
                           const QRectF& rect,
                           DrawImageMode mode,
                           const QRectF& sourceRect) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  window->drawImage(*pixmap, rect, sourceRect, mode);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDrawImageAlpha(string_view windowName,
                                string_view imageID,
                                const QRectF& rect,
                                qreal opacity,
                                const QPointF& origin) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  window->drawImage(*pixmap,
                    rect,
                    QRectF(origin, pixmap->rect().bottomRight().toPointF()),
                    opacity);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowEllipse(string_view windowName,
                         const QRectF& rect,
                         const QPen& pen,
                         const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawEllipse(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowEllipse(string_view windowName,
                         const QRectF& rect,
                         const QPen& pen,
                         const QColor& brushColor,
                         string_view imageID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  QBrush brush(brushColor, *pixmap);
  if (pixmap->depth() != 1) {
    window->drawEllipse(rect, pen, brush);
    return ApiCode::OK;
  }
  window->drawEllipse(rect, brush);
  brush.setColor(pen.color());
  brush.setTexture(image::invertBitmap(*pixmap));
  window->drawEllipse(rect, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowFilter(string_view windowName,
                        const ImageFilter& filter,
                        const QRect& rect) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->applyFilter(filter, rect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowFont(string_view windowName,
                      string_view fontID,
                      const QFont& font) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->loadFont(fontID, font);
  return ApiCode::OK;
}

std::vector<string_view>
ScriptApi::WindowFontList(string_view windowName) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  return window->fontList();
}

ApiCode
ScriptApi::WindowFrame(string_view windowName,
                       const QRectF& rect,
                       const QColor& color1,
                       const QColor& color2) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawFrame(rect, color1, color2);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowGetImageAlpha(string_view windowName,
                               string_view imageID,
                               const QRectF& rect,
                               const QPointF& point) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  QImage alphaImage = pixmap->toImage();
  alphaImage.convertTo(QImage::Format::Format_Alpha8);
  alphaImage.reinterpretAsFormat(QImage::Format::Format_Grayscale8);
  QPixmap alphaPixmap = QPixmap::fromImage(alphaImage);
  window->drawImage(alphaPixmap, rect, QRectF(point, rect.size()));
  return ApiCode::OK;
}

optional<QColor>
ScriptApi::WindowGetPixel(string_view windowName, const QPoint& point) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  const QPixmap& pixmap = window->getPixmap();
  if (!pixmap.rect().contains(point)) {
    return QColor();
  }
  return pixmap.toImage().pixelColor(point);
}

ApiCode
ScriptApi::WindowGradient(string_view windowName,
                          const QRectF& rect,
                          const QColor& color1,
                          const QColor& color2,
                          Qt::Orientation direction) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const qreal horizontalStop =
    direction == Qt::Orientation::Horizontal ? 1.0 : 0.0;
  QLinearGradient gradient(0.0, 0.0, horizontalStop, 1.0 - horizontalStop);
  gradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
  gradient.setColorAt(0.0, color1);
  gradient.setColorAt(1.0, color2);
  window->drawGradient(rect, gradient);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowImageFromWindow(string_view windowName,
                                 string_view imageID,
                                 string_view sourceWindow) const
{
  MiniWindow* window = findWindow(windowName);
  MiniWindow* source = findWindow(sourceWindow);
  if ((window == nullptr) || (source == nullptr)) [[unlikely]] {
    return ApiCode::NoSuchWindow;
  }
  window->loadImage(imageID, source->getPixmap());
  return ApiCode::OK;
}

std::vector<string_view>
ScriptApi::WindowImageList(string_view windowName) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  return window->imageList();
}

QVariant
ScriptApi::WindowInfo(string_view windowName, int64_t infoType) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  return window->info(infoType);
}

ApiCode
ScriptApi::WindowInvert(string_view windowName, const QRect& rect) const
{

  MiniWindow* window = TRY_WINDOW(windowName);
  window->invert(rect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowLine(string_view windowName,
                      const QLineF& line,
                      const QPen& pen) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawLine(line, pen);
  return ApiCode::OK;
}

std::vector<string_view>
ScriptApi::WindowList() const noexcept
{
  return windows.keys();
}

ApiCode
ScriptApi::WindowLoadImage(string_view windowName,
                           string_view imageID,
                           const QString& filename) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  QPixmap image(filename);
  if (!image.isNull()) [[likely]] {
    window->loadImage(imageID, std::move(image));
    return ApiCode::OK;
  }
  if (QFile::exists(filename)) {
    return ApiCode::UnableToLoadImage;
  }
  return ApiCode::FileNotFound;
}

ApiCode
ScriptApi::WindowLoadImageMemory(string_view windowName,
                                 string_view imageID,
                                 QByteArrayView data,
                                 bool swapBlueAndAlpha) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  QImage image = QImage::fromData(data, "PNG");
  if (image.isNull()) [[unlikely]] {
    image = QImage::fromData(data);
    if (image.isNull()) [[unlikely]] {
      return ApiCode::UnableToLoadImage;
    }
  }
  if (swapBlueAndAlpha) {
    ImageFilter::SwapBlueAndAlpha().apply(image);
  }

  window->loadImage(imageID, QPixmap::fromImage(image));
  return ApiCode::OK;
}

QString
ScriptApi::WindowMenu(string_view windowName,
                      const QPoint& location,
                      string_view menuString) const
{
  MiniWindow* window = findWindow(windowName);
  if (window == nullptr) {
    return QString();
  }
  return window->execMenu(location, menuString);
}

ApiCode
ScriptApi::WindowMergeImageAlpha(string_view windowName,
                                 string_view imageID,
                                 string_view maskID,
                                 const QRect& targetRect,
                                 MergeMode mode,
                                 qreal opacity,
                                 const QRect& sourceRect) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = window->findImage(imageID);
  const QPixmap* mask = window->findImage(maskID);
  if (pixmap == nullptr || mask == nullptr) {
    return ApiCode::ImageNotInstalled;
  }
  return window->mergeImageAlpha(
           *pixmap, *mask, targetRect, sourceRect, opacity, mode)
           ? ApiCode::OK
           : ApiCode::BadParameter;
}

ApiCode
ScriptApi::WindowPolygon(string_view windowName,
                         const QPolygonF& polygon,
                         const QPen& pen,
                         const QBrush& brush,
                         bool close,
                         Qt::FillRule fillRule) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  if (brush.style() == Qt::BrushStyle::NoBrush && !close) {
    window->drawPolyline(polygon, pen);
  } else {
    window->drawPolygon(polygon, pen, brush, fillRule);
  }
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowPosition(string_view windowName,
                          const QPoint& location,
                          MiniWindow::Position position,
                          MiniWindow::Flags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setPosition(location, position, flags);
  window->updatePosition();
  stackWindow(windowName, *window);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowRect(string_view windowName,
                      const QRectF& rect,
                      const QPen& pen,
                      const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawRect(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowRect(string_view windowName,
                      const QRectF& rect,
                      const QPen& pen,
                      const QColor& brushColor,
                      string_view imageID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  QBrush brush(brushColor, *pixmap);
  if (pixmap->depth() != 1) {
    window->drawRect(rect, pen, brush);
    return ApiCode::OK;
  }
  window->drawRect(rect, brush);
  brush.setColor(pen.color());
  brush.setTexture(image::invertBitmap(*pixmap));
  window->drawRect(rect, brush);
  return ApiCode::OK;
};

ApiCode
ScriptApi::WindowRoundedRect(string_view windowName,
                             const QRectF& rect,
                             qreal xRadius,
                             qreal yRadius,
                             const QPen& pen,
                             const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawRoundedRect(rect, xRadius, yRadius, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowRoundedRect(string_view windowName,
                             const QRectF& rect,
                             qreal xRadius,
                             qreal yRadius,
                             const QPen& pen,
                             const QColor& brushColor,
                             string_view imageID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  QBrush brush(brushColor, *pixmap);
  if (pixmap->depth() != 1) {
    window->drawRoundedRect(rect, xRadius, yRadius, pen, brush);
    return ApiCode::OK;
  }
  window->drawRoundedRect(rect, xRadius, yRadius, brush);
  brush.setColor(pen.color());
  brush.setTexture(image::invertBitmap(*pixmap));
  window->drawRoundedRect(rect, xRadius, yRadius, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowResize(string_view windowName,
                        const QSize& size,
                        const QColor& fill) const
{
  if (windowName.empty()) {
    return ApiCode::NoNameSpecified;
  }
  if (!size.isValid()) {
    return ApiCode::BadParameter;
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setSize(size, fill);
  window->updatePosition();
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowSetPixel(string_view windowName,
                          const QPoint& point,
                          const QColor& color) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setPixel(point, color);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowSetZOrder(string_view windowName, int64_t order) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setZOrder(order);
  stackWindow(windowName, *window);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowShow(string_view windowName, bool show) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setVisible(show);
  return ApiCode::OK;
}

qreal
ScriptApi::WindowText(string_view windowName,
                      string_view fontID,
                      string_view text,
                      const QRectF& rect,
                      const QColor& color,
                      bool unicode) const
{
  MiniWindow* window = findWindow(windowName);
  if (window == nullptr) [[unlikely]] {
    return -1;
  }
  const QFont* font = window->findFont(fontID);
  if (font == nullptr) [[unlikely]] {
    return -2;
  }
  const QString qtext =
    unicode ? QString::fromUtf8(text) : QString::fromLatin1(text);
  return window->drawText(*font, qtext, rect, color).width();
}

qreal
ScriptApi::WindowTextWidth(string_view windowName,
                           string_view fontID,
                           string_view text,
                           bool unicode) const
{
  MiniWindow* window = findWindow(windowName);
  if (window == nullptr) [[unlikely]] {
    return -1;
  }
  const QFont* font = window->findFont(fontID);
  if (font == nullptr) [[unlikely]] {
    return -2;
  }
  QFontMetricsF fm(*font);
  const QString qtext =
    unicode ? QString::fromUtf8(text) : QString::fromLatin1(text);
  return fm.horizontalAdvance(qtext);
}

ApiCode
ScriptApi::WindowTransformImage(string_view windowName,
                                string_view imageID,
                                MergeMode mode,
                                const QTransform& transform) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* pixmap = TRY_PIXMAP(window, imageID);
  window->drawImage(*pixmap, transform, mode);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowUnloadFont(string_view windowName, string_view fontID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->unloadFont(fontID);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowUnloadImage(string_view windowName, string_view windowID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->unloadImage(windowID);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowWrite(string_view windowName, const QString& filename) const
{
  if (filename.isEmpty()) {
    return ApiCode::NoNameSpecified;
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap& pixmap = window->getPixmap();
  return pixmap.save(filename) ? ApiCode::OK : ApiCode::CouldNotOpenFile;
}
