import AppKit
import DequeModule
import SwiftUI

// SAFETY: Ensured by mutex locks in Rust implementation.
extension RustMudBridgeRef: @unchecked Sendable {}
// SAFETY: Protected here, not used elsewhere.
extension RustOutputStream: @unchecked Sendable {}
// SAFETY: Rust strings are immutable.
extension RustStringRef: @unchecked Sendable {}

class DocumentViewController: NSViewController {
  @IBOutlet private weak var scrollView: NSScrollView!
  @IBOutlet private weak var statusBar: NSVisualEffectView!
  @IBOutlet private weak var splitView: NSSplitView!
  @IBOutlet private weak var inputField: NSTextField!
  @IBOutlet private weak var textView: NSTextView!
  private var history: Deque<String> = []
  private var historyLimit: Int = Int.max

  private var bridge: RustMudBridge?
  private var connectTask: Task<(), Never>?
  let status: StatusBarState = StatusBarState()
  weak var textStorage: NSTextStorage!

  var formatter: OutputFormatter = OutputFormatter()
  var willBreak = false

  func applyWorld(_ world: WorldModel) {
    historyLimit = Int(world.history_lines)
    trimHistory()
    formatter = OutputFormatter(world)

    let style = NSMutableParagraphStyle()
    style.lineSpacing = CGFloat(world.line_spacing)
    textView.defaultParagraphStyle = style

    let textInset = Int(world.pixel_offset)
    textView.textContainerInset = NSSize(width: textInset, height: textInset)

    inputField.textColor = world.input_colors.foreground
    inputField.backgroundColor = world.input_colors.background

    if let bridge = bridge {
      bridge.set_world(World(world))
    } else {
      bridge = RustMudBridge(World(world))
    }
  }

  func trimHistory() {
    if history.count >= historyLimit {
      history.removeFirst(historyLimit - 1 - history.count)
    }
  }

  override func viewDidLoad() {
    super.viewDidLoad()
    initStatusBar()
    textStorage = textView.textStorage
    textView.linkTextAttributes = [:]

    if let welcomeWindow = WelcomeHostingController.window {
      welcomeWindow.close()
    }
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

  func sendInput(_ input: String, fromUser: Bool = false) {
    willBreak = true
    guard let bridge = bridge else {
      return
    }
    if fromUser {
      let outcome = bridge.alias(input)
      if outcome.should_remember() && history.last != input {
        history.append(input)
        trimHistory()
      }
      var shouldScrollToBottom = false
      if outcome.should_display(), let formatted = formatter.formatInput(input) {
        shouldScrollToBottom = true
        textStorage.append(formatted)
      }
      let sendStream = outcome.stream()
      while let send = sendStream.next() {
        if let output = handleSend(send) {
          textStorage.append(output)
          shouldScrollToBottom = true
        }
      }
      if shouldScrollToBottom {
        scrollToBottom()
      }
      if !outcome.should_send() {
        return
      }
    } else if let formatted = formatter.formatInput(input) {
      textStorage.append(formatted)
      scrollToBottom()
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
