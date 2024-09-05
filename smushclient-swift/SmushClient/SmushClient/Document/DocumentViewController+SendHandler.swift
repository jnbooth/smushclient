import AppKit

extension DocumentViewController {
  func handleSend(_ send: SendRequest) -> NSAttributedString? {
    let text = send.text.toString()
    switch send.send_to {
    case .Command:
      setInput(text)

    case .Output:
      let attributes = formatter.plainAttributes
      return NSAttributedString(string: "\n" + text, attributes: attributes)

    case .Status:
      status.pluginMessage = text

    case .World, .WorldDelay, .WorldImmediate:
      sendInput(text)

    default:
      break
    }

    return nil
  }
}
