import AppKit

@main
class AppDelegate: NSObject {
  private var hasStarted = false
}

extension AppDelegate: NSApplicationDelegate {
  func applicationDidFinishLaunching(_ aNotification: Notification) {
    hasStarted = true
  }

  func applicationWillTerminate(_ aNotification: Notification) {
  }

  func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
    .terminateNow
  }

  func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
    true
  }

  func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
    if hasStarted {
      return true
    }
    let controller = NSDocumentController.shared
    let documents = controller.recentDocumentURLs
    guard let lastDocument = documents.first else {
      return true
    }
    controller.openDocument(
      withContentsOf: lastDocument,
      display: true,
      completionHandler: { _, _, error in
        if let error = error {
          NSAlert(error: error).runModal()
          do {
            try controller.openUntitledDocumentAndDisplay(true)
          } catch {
            NSAlert(error: error).runModal()
          }
        }
      })
    return false
  }
}
