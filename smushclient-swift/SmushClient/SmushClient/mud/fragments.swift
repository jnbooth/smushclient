import AppKit

extension NSAttributedString.Key {
  static let choices: NSAttributedString.Key = .init("choices")
  static let sendto: NSAttributedString.Key = .init("sendto")
}

enum RenderError: Error {
  case InvalidUtf8
}

func rgb(_ hex: UInt32) -> NSColor {
  NSColor(
    red: CGFloat(Double((hex >> 16) & 0xFF) / 255),
    green: CGFloat(Double((hex >> 8) & 0xFF) / 255),
    blue: CGFloat(Double(hex & 0xFF) / 255),
    alpha: CGFloat(1)
  )
}

func getColor(_ color: MudColor, _ ansiColors: AnsiColors) -> NSColor {
  switch color {
  case .Ansi(let code): return ansiColors[Int(code)]
  case .Hex(let hex): return rgb(hex)
  }
}

func isBlack(_ color: MudColor) -> Bool {
  switch color {
  case .Ansi(let code): code == 0
  case .Hex(let hex): hex == 0
  }
}

func renderText(_ fragment: RustTextFragment, _ ansiColors: AnsiColors) -> NSAttributedString {
  guard let text = String(bytes: fragment.text(), encoding: .utf8) else {
    return NSAttributedString("�")
  }

  let font = NSFont.monospacedSystemFont(
    ofSize: NSFont.systemFontSize, weight: fragment.isBold() ? .bold : .medium)

  let invert = fragment.isInverse()
  let foreground = invert ? fragment.background() : fragment.foreground()
  let background = invert ? fragment.foreground() : fragment.background()

  var attrs: [NSAttributedString.Key: Any] = [
    .font: font,
    .foregroundColor: getColor(foreground, ansiColors),
  ]

  if !isBlack(background) {
    attrs[.backgroundColor] = getColor(background, ansiColors)
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
