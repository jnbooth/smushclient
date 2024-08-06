import AppKit

class StatusBarState: NSObject {
  @objc dynamic var connectedColor: NSColor = .gray
  @objc dynamic var connectedText: String = "Disconnected"
  @objc dynamic var connected = false {
    didSet {
      if connected {
        connectedColor = .lightGray
        connectedText = "Connected"
      } else {
        connectedColor = .gray
        connectedText = "Disconnected"
      }
    }
  }

}
