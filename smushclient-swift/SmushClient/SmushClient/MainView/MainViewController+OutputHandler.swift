import AppKit
import SwiftUI

extension MainViewController {
  func handleOutput(_ fragment: OutputFragment) {
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
      appendText(text)
    case .Send(_):
      return
    case .Sound(_):
      return
    }
    scrollToBottom()
  }

  private func handleBell() {
    Task {
      let _ = await MainActor.run {
        NSApplication.shared.requestUserAttention(.criticalRequest)
      }
    }
  }

  private func handleBreak() {
    if willBreak {
      appendText(NSAttributedString("\n"))
      willBreak = false
    }
  }
}
