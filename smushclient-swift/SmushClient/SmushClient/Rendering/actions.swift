import AppKit

let sendToInputSuffix = "\u{17}"
let sendToInputChar = Character(sendToInputSuffix)

extension NSAttributedString.Key {
  static let choices: NSAttributedString.Key = .init("choices")
}

enum InternalSendTo {
  case Input
  case World
}

func setupActionAttributes(
  link: RustMxpLinkRef,
  text: String,
  attributes: inout [NSAttributedString.Key: Any]
) {
  let action = link.action().toString()
  let send_to = link.send_to()
  switch send_to {
  case .Input:
    attributes[.link] = action + sendToInputSuffix
  case .Internet:
    if let url = URL(string: action) {
      attributes[.link] = url
    }
  case .World:
    attributes[.link] = action
  }
  attributes[.toolTip] = action
  let prompts = link.prompts()
  if prompts.isEmpty {
    attributes[.cursor] = NSCursor.pointingHand
    return
  }
  attributes[.cursor] = NSCursor.contextualMenu
  var choices: [String] = []
  for prompt in prompts {
    choices.append(prompt.as_str().toString())
  }
  attributes[.choices] = choices
}

func deserializeActionUrl(_ url: String) -> (InternalSendTo, String) {
  if url.last == sendToInputChar {
    return (.Input, String(url.dropLast()))
  }
  return (.World, url)
}

func mxpActionMenu(attributes: [NSAttributedString.Key: Any], action: Selector) -> NSMenu? {
  guard let actionUrl = attributes[.link] as? String else {
    return nil
  }
  let menu = NSMenu()
  menu.addItem(actionMenuItem(actionUrl, action))
  guard let choices = attributes[.choices] as? [String] else {
    return menu
  }
  for choice in choices {
    menu.addItem(actionMenuItem(choice, action))
  }
  return menu
}

private func actionMenuItem(_ send: String, _ action: Selector) -> NSMenuItem {
  let item = NSMenuItem()
  item.title = send
  item.action = action
  return item
}
