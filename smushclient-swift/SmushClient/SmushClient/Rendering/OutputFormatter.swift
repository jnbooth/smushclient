import AppKit

extension RgbColor {
  func isBlack() -> Bool {
    self.r == 0 && self.g == 0 && self.b == 0
  }
}

struct OutputFormatter {
  private let fonts: OutputFonts
  let plainAttributes: [NSAttributedString.Key: Any]

  init() {
    fonts = OutputFonts()
    plainAttributes = [.font: fonts.provide()]
  }

  init(_ world: WorldModel) {
    fonts = OutputFonts(world)
    plainAttributes = [.font: fonts.provide()]
  }

  func format(_ fragment: RustTextFragment) -> NSAttributedString {
    let text = fragment.text().toString()

    var attrs: [NSAttributedString.Key: Any] = [
      .font: fonts.provide(bold: fragment.isBold(), italic: fragment.isItalic()),
      .foregroundColor: NSColor(fragment.foreground()),
    ]

    let background = fragment.background()
    if !background.isBlack() {
      attrs[.backgroundColor] = NSColor(background)
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
}
