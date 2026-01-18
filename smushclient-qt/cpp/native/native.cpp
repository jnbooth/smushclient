#include "native.h"
#include "window.h"

SetBackgroundMaterialResult
native::setBackgroundMaterial(QWidget* widget, int material)
{
  widget->setAttribute(Qt::WA_NoSystemBackground);
  widget->setAttribute(Qt::WA_TranslucentBackground);
  void* wid = reinterpret_cast<void*>(widget->winId());
#if defined(Q_OS_MACOS)
  return SetEffectViewBackground(wid, material);
#else
  return SetBackgroundMaterialResult::Unsupported;
#endif
}

SetBackgroundMaterialResult
native::unsetBackgroundMaterial(QWidget* widget)
{
  if (!widget->testAttribute(Qt::WA_NativeWindow))
    return SetBackgroundMaterialResult::OK;

  widget->setAttribute(Qt::WA_TranslucentBackground, false);
  widget->setAttribute(Qt::WA_NoSystemBackground, false);
  void* wid = reinterpret_cast<void*>(widget->winId());
#if defined(Q_OS_MACOS)
  return UnsetEffectViewBackground(wid);
#else
  return SetBackgroundMaterialResult::Unsupported;
#endif
}
