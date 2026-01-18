#pragma once
#include "result.h"

using SetBackgroundMaterialResult = native::SetBackgroundMaterialResult;

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_OSX
#endif

#ifdef PLATFORM_OSX
extern "C"
{
  // nativeViewPtr is a void* (cast from NSView*)
  SetBackgroundMaterialResult SetEffectViewBackground(void* nativeViewPtr,
                                                      int material);
  SetBackgroundMaterialResult UnsetEffectViewBackground(void* nativeViewPtr);
}
#endif
