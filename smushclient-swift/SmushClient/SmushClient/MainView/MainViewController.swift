import AppKit
import SwiftUI

extension RustMudBridgeRef: @unchecked Sendable {}
extension RustOutputStream: @unchecked Sendable {}
extension RustStringRef: @unchecked Sendable {}

struct NotificationName {
  static let NewWorld = NSNotification.Name("newWorld")
}

class MainViewController: NSViewController {
  @IBOutlet private weak var scrollView: NSScrollView!
  @IBOutlet private weak var statusBar: NSVisualEffectView!
  @IBOutlet private weak var splitView: NSSplitView!
  @IBOutlet private weak var inputField: NSTextField!
  @IBOutlet private weak var textView: NSTextView!
  var settingsWindowController: NSWindowController!

  private var bridge: RustMudBridge?
  private var connectTask: Task<(), Never>?
  private let defaults = AppDefaults()
  private let status: StatusBarState = StatusBarState()
  private weak var textStorage: NSTextStorage!

  private var inputFormatter: InputFormatter = InputFormatter()
  private var outputFormatter: OutputFormatter = OutputFormatter()
  var willBreak = false

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

  override func viewDidLoad() {
    super.viewDidLoad()

    initStatusBar()
    textStorage = textView.textStorage
    NotificationCenter.default.addObserver(
      self, selector: #selector(startWorld), name: NotificationName.NewWorld, object: nil)
  }

  private func initStatusBar() {
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

  @IBAction private func showWorldSettings(_ sender: Any?) {
    settingsWindowController.showWindow(self)
  }

  @objc private func startWorld(_ notification: Notification) {
    connect()
  }

  func handleError(_ error: Error) {
    NSAlert(error: error).runModal()
  }

  func setInput(_ input: String) {
    inputField.stringValue = input
  }

  func scrollToBottom() {
    textView.scrollRangeToVisible(NSRange(location: textStorage.length, length: 0))
  }

  func sendInput(_ input: String) {
    if let formatted = inputFormatter.format(input) {
      textStorage.append(formatted)
      scrollToBottom()
    }
    willBreak = true
    guard let bridge = bridge else {
      return
    }
    Task {
      do {
        try await bridge.send(input + "\r\n")
      } catch {
        handleError(error)
      }
    }
  }

  func appendText(_ fragment: RustTextFragment) {
    textStorage.append(outputFormatter.format(fragment))
  }

  func appendText(_ string: NSAttributedString) {
    textStorage.append(string)
  }

  func connect() {
    guard let bridge = bridge else {
      return
    }
    if bridge.connected() {
      return
    }
    connectTask = Task {
      do {
        try await bridge.connect()
        status.connected = true
        while true {
          let fragments = try await bridge.receive()
          while let fragment = fragments.next() {
            handleOutput(fragment)
          }
        }
      } catch {
        status.connected = false
        handleError(error)
      }
    }
  }

  func disconnect() async throws {
    if let connectTask = connectTask {
      connectTask.cancel()
    }
    if let bridge = bridge {
      _ = try await bridge.disconnect()
    }
    status.connected = false
  }
}
