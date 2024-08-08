import AppKit

private let documentScene = NSStoryboard.SceneIdentifier("Document Window Controller")
private let worldScene = NSStoryboard.SceneIdentifier("World Settings Window Controller")

class SmushfileDocument: NSDocument {
  private var content: WorldModel!
  private weak var viewController: MainViewController!
  private weak var settingsController: NSWindowController!
  private var closing = false

  override init() {
    super.init()
    hasUndoManager = true
  }

  override func makeWindowControllers() {
    let isNew = content == nil
    if isNew {
      content = WorldModel()
    }
    
    let storyboard = NSStoryboard(name: NSStoryboard.Name("Main"), bundle: nil)

    let windowController =
      storyboard.instantiateController(withIdentifier: documentScene) as! NSWindowController
    windowController.shouldCascadeWindows = true
    addWindowController(windowController)

    let mainController = windowController.contentViewController as! MainViewController
    viewController = mainController
    
    
    WorldSettingsHostingController.targetWorld = content
    let settingsWindowController =
      storyboard.instantiateController(withIdentifier: worldScene) as! NSWindowController
    WorldSettingsHostingController.targetWorld = nil
    addWindowController(settingsWindowController)
    settingsController = settingsWindowController


    if isNew {
      showWorldSettings()
    } else {
      mainController.applyWorld(content)
      mainController.connect()
    }
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
  
  func setContent(_ world: WorldModel) {
    updateChangeCount(.changeDone)
    content = world
    viewController.applyWorld(world)
  }
  
  @IBAction private func showWorldSettings(_ sender: Any? = nil) {
    settingsController.showWindow(self)
  }
}

extension SmushfileDocument: NSWindowDelegate {
  func windowWillClose(_ notification: Notification) {
    if closing {
      return
    }
    let manager = undoManager!
    if manager.canUndo {
      viewController.applyWorld(content)
      manager.removeAllActions()
      return
    }
    if isDraft && !isDocumentEdited {
      closing = true
      close()
    }
    
  }
}
