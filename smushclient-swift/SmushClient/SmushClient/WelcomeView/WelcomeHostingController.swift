import SwiftUI

class WelcomeHostingController: NSHostingController<WelcomeView> {
  static weak var window: NSWindow?

  required init?(coder: NSCoder) {
    let recent = NSDocumentController.shared.recentDocumentURLs.map(RecentDocument.init)
    super.init(coder: coder, rootView: WelcomeView(recentDocuments: recent))
  }

  override func viewDidLoad() {
    let window = view.window!
    Self.window = window
    window.delegate = self
  }
}

extension WelcomeHostingController: NSWindowDelegate {
  func windowWillClose(_ notification: Notification) {
    Self.window = nil
    if NSDocumentController.shared.documents.isEmpty {
      NSApplication.shared.terminate(notification)
    }
  }
}
