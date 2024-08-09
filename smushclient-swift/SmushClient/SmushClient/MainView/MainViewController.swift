import AppKit
import SwiftUI

// SAFETY: Ensured by mutex locks in Rust implementation.
extension RustMudBridgeRef: @unchecked Sendable {}
// SAFETY: All it can do is yield .next(), so it's safe for this usage.
extension RustOutputStream: @unchecked Sendable {}
// SAFETY: Rust strings are immutable.
extension RustStringRef: @unchecked Sendable {}

class MainViewController: NSViewController {
  @IBOutlet private weak var scrollView: NSScrollView!
  @IBOutlet private weak var statusBar: NSVisualEffectView!
  @IBOutlet private weak var splitView: NSSplitView!
  @IBOutlet private weak var inputField: NSTextField!
  @IBOutlet private weak var textView: NSTextView!

  private var bridge: RustMudBridge?
  private var connectTask: Task<(), Never>?
  let status: StatusBarState = StatusBarState()
  weak var textStorage: NSTextStorage!

  private var inputFormatter: InputFormatter = InputFormatter()
  var outputFormatter: OutputFormatter = OutputFormatter()
  var willBreak = false

  func applyWorld(_ world: WorldModel) {
    inputFormatter = InputFormatter(world)
    outputFormatter = OutputFormatter(world)

    let style = NSMutableParagraphStyle()
    style.lineSpacing = CGFloat(world.line_spacing)
    textView.defaultParagraphStyle = style

    let textInset = Int(world.pixel_offset)
    textView.textContainerInset = NSSize(width: textInset, height: textInset)

    if let bridge = bridge {
      bridge.set_world(World(world))
    } else {
      bridge = RustMudBridge(World(world))
    }
  }

  override func viewDidLoad() {
    super.viewDidLoad()
    initStatusBar()
    textStorage = textView.textStorage
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

  func handleError(_ error: Error) {
    NSAlert(error: error).runModal()
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

  func setInput(_ input: String) {
    inputField.stringValue = input
  }

  @IBAction func connect(_ sender: Any? = nil) {
    guard let bridge = bridge else {
      return
    }
    if bridge.connected() {
      return
    }
    if let connectTask = connectTask {
      connectTask.cancel()
    }
    connectTask = Task {
      do {
        try await bridge.connect()
        status.connected = true
        while true {
          let output = try await bridge.receive()
          handleOutput(output)
        }
      } catch {
        status.connected = false
        handleError(error)
      }
    }
  }

  @IBAction func disconnect(_ sender: Any? = nil) {
    if let connectTask = connectTask {
      connectTask.cancel()
    }
    connectTask = Task {
      do {
        if let bridge = bridge {
          _ = try await bridge.disconnect()
        }
        status.connected = false
      } catch {
        handleError(error)
      }
    }
  }
}
