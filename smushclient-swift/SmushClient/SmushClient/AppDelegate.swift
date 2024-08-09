import AppKit

@main
class AppDelegate: NSObject {}

extension AppDelegate: NSApplicationDelegate {
  func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
    .terminateNow
  }

  func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
    true
  }

  func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
    Stories().instantiateWelcomeScreen().window.showWindow(self)
    return false
  }

  func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
    applicationShouldOpenUntitledFile(sender)
  }

  func application(_ application: NSApplication, open urls: [URL]) {
    for url in urls {
      NSDocumentController.shared.openDocument(withContentsOf: url, display: true) { _, _, error in
        if let error = error {
          NSAlert(error: error).runModal()
        }
      }
    }
  }
}
