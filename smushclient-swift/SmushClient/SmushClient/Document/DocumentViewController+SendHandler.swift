import AppKit

extension DocumentViewController {
  func handleSend(_ send: SendRequest) -> Bool {
    let text = send.text.toString()
    switch send.send_to {
    case .Command:
      setInput(text)

    case .Output:
      let attributes = outputFormatter.plainAttributes
      textStorage.append(NSAttributedString(string: text, attributes: attributes))
      return true

    case .Status:
      status.pluginMessage = text

    case .World, .WorldDelay, .WorldImmediate:
      sendInput(text)

    default:
      break
    }

    return false
  }
}
