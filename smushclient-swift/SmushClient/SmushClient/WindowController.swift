import Cocoa

class WindowController: NSWindowController, NSWindowDelegate {

  override func windowDidLoad() {
    super.windowDidLoad()
  }

  required init?(coder aDecoder: NSCoder) {
    super.init(coder: aDecoder)
    shouldCascadeWindows = true
  }

}
