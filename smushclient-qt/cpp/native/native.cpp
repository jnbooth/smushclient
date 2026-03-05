#include "native.h"
#include "window.h"

#if defined(Q_OS_MACOS)
namespace {
inline void*
getWID(QWidget* widget)
{
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  // SAFETY: QWidget::winId() returns an NSView pointer on Mac OS
  return reinterpret_cast<void*>(widget->winId());
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}
} // namespace
#endif

SetBackgroundMaterialResult
native::setBackgroundMaterial(QWidget* widget, int material)
{
  widget->setAttribute(Qt::WA_NoSystemBackground);
  widget->setAttribute(Qt::WA_TranslucentBackground);
#if defined(Q_OS_MACOS)
  return SetEffectViewBackground(getWID(widget), material);
#else
  return SetBackgroundMaterialResult::Unsupported;
#endif
}

SetBackgroundMaterialResult
native::unsetBackgroundMaterial(QWidget* widget)
{
  if (!widget->testAttribute(Qt::WA_NativeWindow)) {
    return SetBackgroundMaterialResult::OK;
  }

  widget->setAttribute(Qt::WA_TranslucentBackground, false);
  widget->setAttribute(Qt::WA_NoSystemBackground, false);
#if defined(Q_OS_MACOS)
  return UnsetEffectViewBackground(getWID(widget));
#else
  return SetBackgroundMaterialResult::Unsupported;
#endif
}
