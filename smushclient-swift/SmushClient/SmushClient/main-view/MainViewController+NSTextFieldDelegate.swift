import AppKit
import SwiftUI

extension MainViewController: NSTextViewDelegate {
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
}
