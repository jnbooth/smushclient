import AppKit

private let documentScene = NSStoryboard.SceneIdentifier("Document Window Controller")
private let worldScene = NSStoryboard.SceneIdentifier("World Settings Window Controller")

class SmushfileDocument: NSDocument {
  var content: WorldModel
  var viewController: MainViewController!

  override init() {
    content = WorldModel()
    super.init()
  }

  override func makeWindowControllers() {
    let storyboard = NSStoryboard(name: NSStoryboard.Name("Main"), bundle: nil)

    WorldSettingsHostingController.targetWorld = content
    let settingsWindowController =
      storyboard.instantiateController(withIdentifier: worldScene) as! NSWindowController
    WorldSettingsHostingController.targetWorld = nil
    addWindowController(settingsWindowController)

    let windowController =
      storyboard.instantiateController(withIdentifier: documentScene) as! NSWindowController
    windowController.shouldCascadeWindows = true
    addWindowController(windowController)

    let mainController = windowController.contentViewController as! MainViewController
    mainController.settingsWindowController = settingsWindowController
    mainController.world = content
    viewController = mainController

    if !content.site.isEmpty {
      settingsWindowController.close()
      mainController.connect()
    }

    settingsWindowController.window!.delegate = self
  }

  override func canAsynchronouslyWrite(
    to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType
  ) -> Bool {
    true
  }

  override func read(from data: Data, ofType typeName: String) throws {
    try MainActor.assumeIsolated {
      let world = try data.withUnsafeBytes { body in
        try read_world(body.assumingMemoryBound(to: UInt8.self))
      }
      content = WorldModel(world)
    }
  }

  override func data(ofType typeName: String) throws -> Data {
    let vec = try write_world(World(content))
    let len = vec.len()
    var data = Data(capacity: len)
    data.append(vec.as_ptr(), count: len)
    return data
  }
}

extension SmushfileDocument: NSWindowDelegate {
  func windowWillClose(_ notification: Notification) {
    viewController.applyWorld()
  }
}
