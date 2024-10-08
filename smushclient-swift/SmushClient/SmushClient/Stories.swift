import AppKit

struct Story<T> {
  let window: NSWindowController
  let view: T
}

struct Stories {
  private let storyboard: NSStoryboard

  init(_ name: NSStoryboard.Name = "Main", bundle: Bundle? = nil) {
    storyboard = NSStoryboard(name: NSStoryboard.Name(name), bundle: bundle)
  }

  @MainActor func instantiateDocument() -> Story<DocumentViewController> {
    instantiate("Document Window Controller")
  }

  @MainActor func instantiateWelcomeScreen() -> Story<WelcomeHostingController> {
    instantiate("Welcome Window Controller")
  }

  @MainActor func instantiateWorldSettings(_ world: WorldModel) -> Story<
    WorldSettingsHostingController
  > {
    WorldSettingsHostingController.targetWorld = world
    defer {
      WorldSettingsHostingController.targetWorld = nil
    }
    return instantiate("World Settings Window Controller")
  }

  @MainActor private func instantiate<T>(_ identifier: NSStoryboard.SceneIdentifier) -> Story<T> {
    let window = storyboard.instantiateController(withIdentifier: identifier) as! NSWindowController
    let view = window.contentViewController as! T
    return Story(window: window, view: view)
  }
}
