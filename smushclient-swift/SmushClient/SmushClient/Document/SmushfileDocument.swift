import AppKit

private let documentScene = NSStoryboard.SceneIdentifier("Document Window Controller")
private let worldScene = NSStoryboard.SceneIdentifier("World Settings Window Controller")

class SmushfileDocument: NSDocument {
  private var content: WorldModel!
  private weak var documentWindow: NSWindowController!
  private weak var documentView: DocumentViewController!
  private var isNew = true

  override init() {
    super.init()
    hasUndoManager = true
  }

  override func defaultDraftName() -> String {
    "New World"
  }
  
  private func instantiateSettingsWindow() {
    let worldSettingsScene = Stories().instantiateWorldSettings(content)
    worldSettingsScene.window.window!.delegate = self
    worldSettingsScene.window.showWindow(self)
  }

  private func instantiateDocumentWindow() {
    let documentScene = Stories().instantiateDocument()
    documentWindow = documentScene.window
    documentView = documentScene.view
    addWindowController(documentScene.window)
    
    documentScene.window.shouldCascadeWindows = true
    documentScene.window.shouldCloseDocument = true
  }
  
  private func startWorld() {
    documentWindow.synchronizeWindowTitleWithDocumentName()
    documentView.applyWorld(content)
    documentView.connect()
    let window = documentWindow.window!
    if let screen = window.screen ?? NSScreen.main {
      window.setFrame(screen.visibleFrame, display: true)
    }
  }

  override func makeWindowControllers() {
    if content == nil {
      content = WorldModel()
    } else {
      isNew = false
    }
    instantiateDocumentWindow()
    if isNew {
      instantiateSettingsWindow()
    } else {
      startWorld()
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
        try readWorld(body.assumingMemoryBound(to: UInt8.self))
      }
      content = WorldModel(world)
      displayName = content.name
    }
  }

  override func data(ofType typeName: String) throws -> Data {
    let vec = try writeWorld(World(content))
    let len = vec.len()
    var data = Data(capacity: len)
    data.append(vec.as_ptr(), count: len)
    return data
  }

  @IBAction private func showWorldSettings(_ sender: Any? = nil) {
    instantiateSettingsWindow()
  }
}

extension SmushfileDocument: NSWindowDelegate {
  func windowWillClose(_ notification: Notification) {
    if content.site.isEmpty {
      close()
      return
    }

    displayName = content.name
    
    if content.save_world_automatically {
      autosave(withImplicitCancellability: true) { _ in return }
    }
  
    if isNew {
      startWorld()
      isNew = false
    }
  }
}
