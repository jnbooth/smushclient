import AppKit
import SwiftUI

extension MainViewController {
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
            handleOutput(fragment)
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
}
