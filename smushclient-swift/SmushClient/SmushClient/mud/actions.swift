import AppKit

public enum InternalSendTo {
  case Input
  case World
}

public func serializeActionUrl(_ sendto: SendTo, _ action: String) -> String {
  switch sendto {
  case .Input:
    return "input:" + action
  case .Internet:
    return action
  case .World:
    return "send:" + action
  }
}

public func deserializeActionUrl(_ url: String) -> (InternalSendTo, Substring)? {
  let components = url.split(separator: ":", maxSplits: 1)
  if components.count < 2 {
    return nil
  }
  switch components[0] {
  case "input":
    return (.Input, components[1])
  case "send":
    return (.World, components[1])
  default:
    return nil
  }
}

public func setupActionAttributes(link: RustMxpLinkRef, text: String, attributes: inout  [NSAttributedString.Key: Any]) {
  let action = link.action().toString().replacing("&text;", with: text)
  let sendto = link.sendto();
  if sendto == .Internet, let url = URL(string: action) {
    attributes[.link] = url
  } else {
    attributes[.link] = serializeActionUrl(sendto, action)
  }
  // attrs[.underlineStyle] = NSUnderlineStyle.single
  attributes[.toolTip] = action
  // attrs[.cursor] = NSCursor.pointingHand
  let prompts = link.prompts()
  if prompts.isEmpty {
    return
  }
  var choices: [String] = []
  for prompt in prompts {
    choices.append(prompt.as_str().toString())
  }
  attributes[.choices] = choices
}

func actionMenuItem(_ send: String, _ action: Selector) -> NSMenuItem {
  let item = NSMenuItem()
  item.title = send
  item.action = action
  return item
}

public func mxpActionMenu(attributes: [NSAttributedString.Key : Any], action: Selector) -> NSMenu? {
  guard
    let actionUrl = attributes[.link] as? String,
    let (sendto, mainAction) = deserializeActionUrl(actionUrl),
    case .World = sendto
  else {
    return nil
  }
  let menu = NSMenu()
  menu.addItem(actionMenuItem(String(mainAction), action))
  guard let choices = attributes[.choices] as? [String] else {
    return menu
  }
  for choice in choices {
    menu.addItem(actionMenuItem(choice, action))
  }
  return menu
}
