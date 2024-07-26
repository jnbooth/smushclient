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
          let fragment = try await bridge.getOutput()
          await receiveOutput(fragment)
        }
      } catch {
        handleError(error)
      }
    }
  }

  func disconnect() {
    if let connectTask = connectTask {
      connectTask.cancel()
    }
    _ = bridge.disconnect()
  }

  public func control(
    _ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector
  ) -> Bool {
    if commandSelector != #selector(NSResponder.insertNewline(_:)) {
      return false
    }
    let input = inputField.stringValue
    inputField.stringValue = ""
    do {
      try sendInput(input)
    } catch {
      handleError(error)
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

    do {
      try handleLink(sendto, action)
      return true
    } catch {
      handleError(error)
      return false
    }
  }

  public func textView(_ view: NSTextView, menu: NSMenu, for event: NSEvent, at charIndex: Int) -> NSMenu? {
    let attributes = textStorage.attributes(at: charIndex, effectiveRange: nil)
    return mxpActionMenu(attributes: attributes, action: #selector(handleChoice(_:)))
  }

  func receiveOutput(_ fragment: OutputFragment) async {
    switch fragment {
    case .Effect(.Beep):
      await handleBell()
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
    case .Text(let text):
      handleBreak()
      textStorage.append(renderText(text, ansiColors))
    }
    textView.scrollRangeToVisible(NSRange(location: textStorage.length, length: 0))

  }

  func sendInput(_ input: String) throws {
    textStorage.append(NSAttributedString(string: "\n" + input, attributes: inputAttrs))
    willBreak = true
    try bridge.sendInput(input + "\r\n")
  }

  func handleBell() async {
    let _ = await MainActor.run {
      NSApplication.shared.requestUserAttention(.criticalRequest)
    }
  }

  func handleBreak() {
    if willBreak {
      textStorage.append(NSAttributedString("\n"))
      willBreak = false
    }
  }

  @objc func handleChoice(_ item: NSMenuItem) {
    do {
      try sendInput(item.title)
    } catch {
      handleError(error)
    }
  }

  func handleLink(_ sendto: InternalSendTo, _ text: Substring) throws {
    switch sendto {
    case .Input:
      inputField.stringValue = String(text)
    case .World:
      try sendInput(String(text))
    }
  }
}
