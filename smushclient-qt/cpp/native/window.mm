#include "window.h"

#ifdef PLATFORM_OSX
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#include <map>
#include <objc/message.h>
#include <objc/runtime.h>

static char kBackgroundKey; // NOLINT

#define RUN_ON_MAIN(block)                                                     \
  if ([NSThread isMainThread]) {                                               \
    block();                                                                   \
  } else {                                                                     \
    dispatch_sync(dispatch_get_main_queue(), block);                           \
  }

extern "C" SetBackgroundMaterialResult
SetEffectViewBackground(void* nativeViewPtr, int materialIndex)
{
  static const std::array<NSVisualEffectMaterial, 14> materials{
    NSVisualEffectMaterialTitlebar,
    NSVisualEffectMaterialSelection,
    NSVisualEffectMaterialMenu,
    NSVisualEffectMaterialPopover,
    NSVisualEffectMaterialSidebar,
    NSVisualEffectMaterialHeaderView,
    NSVisualEffectMaterialSheet,
    NSVisualEffectMaterialWindowBackground,
    NSVisualEffectMaterialHUDWindow,
    NSVisualEffectMaterialFullScreenUI,
    NSVisualEffectMaterialToolTip,
    NSVisualEffectMaterialContentBackground,
    NSVisualEffectMaterialUnderWindowBackground,
    NSVisualEffectMaterialUnderPageBackground
  };
  if (materialIndex < 0 || materialIndex >= 14) {
    return SetBackgroundMaterialResult::InvalidMaterial;
  }
  const NSVisualEffectMaterial material = materials.at(materialIndex);

  __block SetBackgroundMaterialResult result =
    SetBackgroundMaterialResult::PassedNullPointer;

  RUN_ON_MAIN(^{
    NSView* rootView = reinterpret_cast<NSView*>(nativeViewPtr);
    if (!rootView) {
      return;
    }

    NSView* container = [rootView superview];

    if (!container) {
      result = SetBackgroundMaterialResult::NoSuperview;
      return;
    }

    NSWindow* win = [rootView window];

    if (win) {
      [win setOpaque:NO];

      win.styleMask |=
        NSWindowStyleMaskFullSizeContentView | NSWindowStyleMaskBorderless;
      win.titlebarAppearsTransparent = YES;
    }

    NSVisualEffectView* backgroundView =
      objc_getAssociatedObject(rootView, &kBackgroundKey);

    if (backgroundView) {
      backgroundView.material = material;
      result = SetBackgroundMaterialResult::OK;
      return;
    }

    NSRect frameRect = [rootView frame];

    backgroundView = [[NSVisualEffectView alloc] initWithFrame:frameRect];
    backgroundView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    backgroundView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    backgroundView.material = material;
    backgroundView.state = NSVisualEffectStateActive;
    backgroundView.wantsLayer = YES;
    backgroundView.layer.masksToBounds = YES;
    [container addSubview:backgroundView
               positioned:NSWindowBelow
               relativeTo:rootView];

    objc_setAssociatedObject(
      rootView, &kBackgroundKey, backgroundView, OBJC_ASSOCIATION_RETAIN);
    result = SetBackgroundMaterialResult::OK;
  });

  return result;
}

extern "C" SetBackgroundMaterialResult
UnsetEffectViewBackground(void* nativeViewPtr)
{
  __block SetBackgroundMaterialResult result =
    SetBackgroundMaterialResult::PassedNullPointer;

  RUN_ON_MAIN(^{
    NSView* rootView = reinterpret_cast<NSView*>(nativeViewPtr);
    if (!rootView) {
      return;
    }

    NSView* container = [rootView superview];

    if (!container) {
      result = SetBackgroundMaterialResult::NoSuperview;
      return;
    }

    NSWindow* win = [rootView window];

    if (win) {
      [win setOpaque:YES];
      win.titlebarAppearsTransparent = NO;
    }

    NSVisualEffectView* backgroundView =
      objc_getAssociatedObject(rootView, &kBackgroundKey);
    if (backgroundView) {
      [backgroundView removeFromSuperview];
    }

    objc_setAssociatedObject(
      rootView, &kBackgroundKey, nil, OBJC_ASSOCIATION_COPY);
    result = SetBackgroundMaterialResult::OK;
  });

  return result;
}

#endif // PLATFORM_OSX
