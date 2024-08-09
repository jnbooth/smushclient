import AppKit

private let mainBoard = NSStoryboard.Name("Main")

private let documentScene = NSStoryboard.SceneIdentifier("Document Window Controller")

private let worldScene = NSStoryboard.SceneIdentifier("World Settings Window Controller")

struct Story<T> {
  let window: NSWindowController
  let view: T
}

struct Stories {
  private let storyboard: NSStoryboard

  init(_ name: String = "Main", bundle: Bundle? = nil) {
    storyboard = NSStoryboard(name: NSStoryboard.Name(name), bundle: bundle)
  }

  @MainActor func instantiateDocument() -> Story<MainViewController> {
    instantiate("Document Window Controller")
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

  @MainActor private func instantiate<T>(_ identifier: String) -> Story<T> {
    let id = NSStoryboard.SceneIdentifier(identifier)
    let window = storyboard.instantiateController(withIdentifier: identifier) as! NSWindowController
    let view = window.contentViewController as! T
    return Story(window: window, view: view)
  }
}
