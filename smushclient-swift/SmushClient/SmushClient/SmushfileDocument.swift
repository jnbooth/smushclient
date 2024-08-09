import AppKit

private let documentScene = NSStoryboard.SceneIdentifier("Document Window Controller")
private let worldScene = NSStoryboard.SceneIdentifier("World Settings Window Controller")

class SmushfileDocument: NSDocument {
  private var content: WorldModel!
  private var documentWindow: NSWindowController?
  private weak var documentView: MainViewController?
  private var settingsWindow: NSWindowController?
  private weak var settingsView: WorldSettingsHostingController?
  private var closing = false

  override init() {
    super.init()
    hasUndoManager = true
  }

  private func instantiateDocumentWindow() {
    let documentScene = Stories().instantiateDocument()
    documentWindow = documentScene.window
    documentView = documentScene.view
    addWindowController(documentScene.window)

    documentScene.window.shouldCascadeWindows = true
    documentScene.window.shouldCloseDocument = true
    documentScene.view.applyWorld(content)
    documentScene.view.connect()
  }

  private func instantiateSettingsWindow(shouldClose: Bool) {
    let worldSettingsScene = Stories().instantiateWorldSettings(content)
    settingsWindow = worldSettingsScene.window
    settingsView = worldSettingsScene.view
    addWindowController(worldSettingsScene.window)

    worldSettingsScene.window.shouldCloseDocument = shouldClose
    if !shouldClose {
      worldSettingsScene.window.showWindow(self)
    }
  }

  override func makeWindowControllers() {
    if content == nil {
      content = WorldModel()
      instantiateSettingsWindow(shouldClose: true)
    } else {
      instantiateDocumentWindow()
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

  @IBAction private func showWorldSettings(_ sender: Any? = nil) {
    if let settingsWindow = settingsWindow {
      settingsWindow.showWindow(self)
    } else {
      instantiateSettingsWindow(shouldClose: false)
    }
  }
}

extension SmushfileDocument: NSWindowDelegate {
  func windowWillClose(_ notification: Notification) {
    if content.site.isEmpty {
      return
    }
    if let documentView = documentView {
      documentView.applyWorld(content)
      if content.save_world_automatically {
        autosave(withImplicitCancellability: true) { error in return }
      }
    } else {
      settingsWindow!.shouldCloseDocument = false
      instantiateDocumentWindow()
    }
  }
}
