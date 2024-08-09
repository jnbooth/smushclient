import AppKit
import SwiftUI

extension DocumentViewController {
  @objc private func handleChoice(_ item: NSMenuItem) {
    sendInput(item.title)
  }
}

extension DocumentViewController: NSTextViewDelegate {
  func control(
    _ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector
  ) -> Bool {
    if commandSelector != #selector(NSResponder.insertNewline(_:)) {
      return false
    }
    let input = control.stringValue
    control.stringValue = ""
    sendInput(input)
    return true
  }

  func textView(_ textView: NSTextView, clickedOnLink link: Any, at charIndex: Int) -> Bool {
    guard
      let actionUrl = link as? String,
      let (sendto, action) = deserializeActionUrl(actionUrl)
    else {
      return false
    }

    switch sendto {
    case .Input:
      setInput(String(action))
    case .World:
      sendInput(String(action))
    }

    return true
  }

  func textView(_ view: NSTextView, menu: NSMenu, for event: NSEvent, at charIndex: Int)
    -> NSMenu?
  {
    mxpActionMenu(
      attributes: view.textStorage!.attributes(at: charIndex, effectiveRange: nil),
      action: #selector(handleChoice(_:))
    )
  }
}

extension DocumentViewController: NSTextFieldDelegate {}
