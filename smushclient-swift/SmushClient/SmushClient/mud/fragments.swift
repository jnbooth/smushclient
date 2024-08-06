import AppKit

extension NSAttributedString.Key {
  static let choices: NSAttributedString.Key = .init("choices")
  static let sendto: NSAttributedString.Key = .init("sendto")
}

enum RenderError: Error {
  case InvalidUtf8
}

extension MudColor {
  func color(_ ansiColors: AnsiColors) -> NSColor {
    switch self {
    case .Ansi(let code): ansiColors[Int(code)]
    case .Hex(let hex): NSColor(hex)
    }
  }

  func isBlack() -> Bool {
    switch self {
    case .Ansi(let code): code == 0
    case .Hex(let hex): hex.r == 0 && hex.g == 0 && hex.b == 0
    }
  }
}

func renderText(_ fragment: RustTextFragment, _ ansiColors: AnsiColors) -> NSAttributedString {
  let text = fragment.text().toString()

  let font = NSFont.monospacedSystemFont(
    ofSize: NSFont.systemFontSize, weight: fragment.isBold() ? .bold : .medium)

  let invert = fragment.isInverse()
  let foreground = invert ? fragment.background() : fragment.foreground()
  let background = invert ? fragment.foreground() : fragment.background()

  var attrs: [NSAttributedString.Key: Any] = [
    .font: font,
    .foregroundColor: foreground.color(ansiColors),
  ]

  if !background.isBlack() {
    attrs[.backgroundColor] = background.color(ansiColors)
  }

  if let link = fragment.link() {
    setupActionAttributes(link: link, text: text, attributes: &attrs)
  }

  if fragment.isStrikeout() {
    attrs[.strikethroughStyle] = NSUnderlineStyle.single
  }

  if fragment.isUnderline() {
    attrs[.underlineStyle] = NSUnderlineStyle.single
  }

  return NSAttributedString(string: text, attributes: attrs)
}
