import AppKit
import SwiftUI

let inputAttrs = [
  NSAttributedString.Key.foregroundColor: NSColor.gray,
  NSAttributedString.Key.font: NSFont.monospacedSystemFont(
    ofSize: NSFont.systemFontSize, weight: .medium),
]

public class MainViewController: NSViewController, NSTextFieldDelegate, NSTextViewDelegate {
  @IBOutlet weak var inputField: NSTextField!
  @IBOutlet weak var scrollView: NSScrollView!
  @IBOutlet weak var splitView: NSSplitView!
  @IBOutlet weak var textView: NSTextView!
  @IBOutlet weak var statusBar: NSVisualEffectView!
  var status: StatusBarState = StatusBarState()
  var settingsWindowController: NSWindowController!
  var bridge: RustMudBridge?

  weak var world: WorldModel! {
    didSet {
      bridge = RustMudBridge(World(world))
    }
  }

  var connectTask: Task<(), Never>?
  weak var textStorage: NSTextStorage!
  var willBreak = false
  let defaults = AppDefaults()

  func handleError(_ error: Error) {
    print(error.localizedDescription)
  }

  override public func viewDidLoad() {
    super.viewDidLoad()

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

    textStorage = textView.textStorage
    NotificationCenter.default.addObserver(
      self, selector: #selector(startWorld), name: NotificationName.NewWorld, object: nil)
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
            receiveOutput(fragment)
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

  public func control(
    _ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector
  ) -> Bool {
    if commandSelector != #selector(NSResponder.insertNewline(_:)) {
      return false
    }
    let input = inputField.stringValue
    inputField.stringValue = ""
    Task {
      do {
        try await sendInput(input)
      } catch {
        handleError(error)
      }
    }
    return true
  }

  public func textView(_ textView: NSTextView, clickedOnLink link: Any, at charIndex: Int) -> Bool {
    guard
      let actionUrl = link as? String,
      let (sendto, action) = deserializeActionUrl(actionUrl)
    else {
      return false
    }

    Task {
      do {
        try await handleLink(sendto, action)
      } catch {
        handleError(error)
      }
    }

    return true
  }

  public func textView(_ view: NSTextView, menu: NSMenu, for event: NSEvent, at charIndex: Int)
    -> NSMenu?
  {
    mxpActionMenu(
      attributes: textStorage.attributes(at: charIndex, effectiveRange: nil),
      action: #selector(handleChoice(_:))
    )
  }

  public func receiveOutput(_ fragment: OutputFragment) {
    switch fragment {
    case .Effect(.Beep):
      handleBell()
      return
    case .Effect(_):
      return
    case .Hr:
      break
    case .Image:
      break
    case .LineBreak:
      handleBreak()
      willBreak = true
    case .PageBreak:
      break
    case .Telnet(_):
      return
    case .Text(let text):
      handleBreak()
      textStorage.append(renderText(text, world!.ansi_colors))
    case .Send(_):
      return
    case .Sound(_):
      return
    }
    textView.scrollRangeToVisible(NSRange(location: textStorage.length, length: 0))

  }

  func sendInput(_ input: String) async throws {
    textStorage.append(NSAttributedString(string: "\n" + input, attributes: inputAttrs))
    willBreak = true
    try await bridge!.send(input + "\r\n")
  }

  func handleBell() {
    Task {
      let _ = await MainActor.run {
        NSApplication.shared.requestUserAttention(.criticalRequest)
      }
    }
  }

  func handleBreak() {
    if willBreak {
      textStorage.append(NSAttributedString("\n"))
      willBreak = false
    }
  }

  @objc func handleChoice(_ item: NSMenuItem) async {
    do {
      try await sendInput(item.title)
    } catch {
      handleError(error)
    }
  }

  func handleLink(_ sendto: InternalSendTo, _ text: Substring) async throws {
    switch sendto {
    case .Input:
      inputField.stringValue = String(text)
    case .World:
      try await sendInput(String(text))
    }
  }
}
