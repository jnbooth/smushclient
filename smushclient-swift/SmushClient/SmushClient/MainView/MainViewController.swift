import AppKit
import SwiftUI

struct NotificationName {
  static let NewWorld = NSNotification.Name("newWorld")
}

public class MainViewController: NSViewController {
  @IBOutlet weak var inputField: NSTextField!
  @IBOutlet weak var scrollView: NSScrollView!
  @IBOutlet weak var splitView: NSSplitView!
  @IBOutlet weak var textView: NSTextView!
  @IBOutlet weak var statusBar: NSVisualEffectView!
  var status: StatusBarState = StatusBarState()
  var settingsWindowController: NSWindowController!
  var bridge: RustMudBridge?
  var inputFormatter: InputFormatter = InputFormatter()
  var outputFormatter: OutputFormatter = OutputFormatter()
  var connectTask: Task<(), Never>?
  weak var textStorage: NSTextStorage!
  var willBreak = false
  let defaults = AppDefaults()

  weak var world: WorldModel! {
    didSet {
      bridge = RustMudBridge(World(world))
      applyWorld()
    }
  }

  func applyWorld() {
    inputFormatter = InputFormatter(world)
    outputFormatter = OutputFormatter(world)
  }

  override public func viewDidLoad() {
    super.viewDidLoad()

    initStatusBar()
    textStorage = textView.textStorage
    NotificationCenter.default.addObserver(
      self, selector: #selector(startWorld), name: NotificationName.NewWorld, object: nil)
  }

  func initStatusBar() {
    let statusView = NSHostingView(rootView: StatusBarView(status: status))
    statusView.translatesAutoresizingMaskIntoConstraints = false
    statusBar.addConstraint(
      NSLayoutConstraint(
        item: statusView,
        attribute: .height,
        relatedBy: .equal,
        toItem: statusBar,
        attribute: .height,
        multiplier: 1,
        constant: 0
      ))
    statusBar.addSubview(statusView)
  }

  deinit {
    NotificationCenter.default.removeObserver(self, name: NotificationName.NewWorld, object: nil)
  }

  @IBAction func showWorldSettings(_ sender: Any?) {
    settingsWindowController.showWindow(self)
  }

  @objc func startWorld(_ notification: Notification) {
    connect()
  }

  @objc func handleChoice(_ item: NSMenuItem) async {
    do {
      try await sendInput(item.title)
    } catch {
      handleError(error)
    }
  }

  func handleError(_ error: Error) {
    NSAlert(error: error).runModal()
  }

  func sendInput(_ input: String) async throws {
    if let formatted = inputFormatter.format(input) {
      textStorage.append(formatted)
    }
    willBreak = true
    try await bridge!.send(input + "\r\n")
  }

}
