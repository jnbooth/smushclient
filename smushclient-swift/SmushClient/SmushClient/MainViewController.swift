import AppKit
import Cocoa

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

  let ansiColors = defaultAnsiColors()
  let bridge = RustMudBridge("discworld.atuin.net", 4242)
  var connectTask: Task<(), Never>?
  weak var textStorage: NSTextStorage!
  var willBreak = false
  let defaults = AppDefaults()

  func handleError(_ error: Error) {
    print(error.localizedDescription)
  }

  override public func viewDidLoad() {
    textStorage = textView.textStorage
    inputField.delegate = self
    textView.delegate = self

    connect()
  }

  func connect() {
    connectTask = Task {
      do {
        try await bridge.connect()
        while true {
          let fragments = try await bridge.receive()
          while let fragment = fragments.next() {
            receiveOutput(fragment)
          }
        }
      } catch {
        handleError(error)
      }
    }
  }

  func disconnect() async throws {
    if let connectTask = connectTask {
      connectTask.cancel()
    }
    _ = try await bridge.disconnect()
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

  public func textView(_ view: NSTextView, menu: NSMenu, for event: NSEvent, at charIndex: Int) -> NSMenu? {
    let attributes = textStorage.attributes(at: charIndex, effectiveRange: nil)
    return mxpActionMenu(attributes: attributes, action: #selector(handleChoice(_:)))
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
      textStorage.append(renderText(text, ansiColors))
    }
    textView.scrollRangeToVisible(NSRange(location: textStorage.length, length: 0))

  }

  func sendInput(_ input: String) async throws {
    textStorage.append(NSAttributedString(string: "\n" + input, attributes: inputAttrs))
    willBreak = true
    try await bridge.send(input + "\r\n")
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
