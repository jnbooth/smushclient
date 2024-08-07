import AppKit
import SwiftUI

extension MainViewController {
  public func handleOutput(_ fragment: OutputFragment) {
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
      textStorage.append(outputFormatter.format(text))
    case .Send(_):
      return
    case .Sound(_):
      return
    }
    textView.scrollRangeToVisible(NSRange(location: textStorage.length, length: 0))
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

  func handleLink(_ sendto: InternalSendTo, _ text: Substring) async throws {
    switch sendto {
    case .Input:
      inputField.stringValue = String(text)
    case .World:
      try await sendInput(String(text))
    }
  }
}
