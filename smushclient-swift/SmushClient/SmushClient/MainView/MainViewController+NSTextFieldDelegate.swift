import AppKit
import SwiftUI

extension MainViewController {
  @objc private func handleChoice(_ item: NSMenuItem) async {
    do {
      try await sendInput(item.title)
    } catch {
      handleError(error)
    }
  }
}

extension MainViewController: NSTextViewDelegate {
  func control(
    _ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector
  ) -> Bool {
    if commandSelector != #selector(NSResponder.insertNewline(_:)) {
      return false
    }
    let input = control.stringValue
    control.stringValue = ""
    Task {
      do {
        try await sendInput(input)
      } catch {
        handleError(error)
      }
    }
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
      Task {
        do {
          try await sendInput(String(action))
        } catch {
          handleError(error)
        }
      }
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

extension MainViewController: NSTextFieldDelegate {}
