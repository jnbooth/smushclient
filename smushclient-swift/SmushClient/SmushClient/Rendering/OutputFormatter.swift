import AppKit

extension RgbColor {
  func isBlack() -> Bool {
    self.r == 0 && self.g == 0 && self.b == 0
  }
}

struct OutputFormatter {
  private let fonts: OutputFonts
  private let showUnderline: Bool
  private let hyperlinkColor: NSColor?
  private let underlineHyperlinks: Bool
  let plainAttributes: [NSAttributedString.Key: Any]

  init() {
    fonts = OutputFonts()
    plainAttributes = [.font: fonts.provide()]
    showUnderline = true
    hyperlinkColor = nil
    underlineHyperlinks = true
  }

  init(_ world: WorldModel) {
    fonts = OutputFonts(world)
    plainAttributes = [.font: fonts.provide()]
    showUnderline = world.show_underline
    hyperlinkColor = world.use_custom_link_color ? world.hyperlink_color : nil
    underlineHyperlinks = world.underline_hyperlinks
  }

  func format(_ fragment: RustTextFragment) -> NSAttributedString {
    let text = fragment.text().toString()

    let invert = fragment.isInverse()
    let foreground = invert ? fragment.background() : fragment.foreground()
    let background = invert ? fragment.foreground() : fragment.background()

    var attrs: [NSAttributedString.Key: Any] = [
      .font: fonts.provide(bold: fragment.isBold(), italic: fragment.isItalic()),
      .foregroundColor: NSColor(foreground),
    ]

    if !background.isBlack() {
      attrs[.backgroundColor] = NSColor(background)
    }

    if let link = fragment.link() {
      setupActionAttributes(link: link, text: text, attributes: &attrs)
      if let color = hyperlinkColor {
        attrs[.foregroundColor] = color
      }
      if underlineHyperlinks {
        attrs[.underlineStyle] = NSUnderlineStyle.single
      }
    }

    if fragment.isStrikeout() {
      attrs[.strikethroughStyle] = NSUnderlineStyle.single
    }

    if showUnderline && fragment.isUnderline() {
      attrs[.underlineStyle] = NSUnderlineStyle.single
    }

    return NSAttributedString(string: text, attributes: attrs)
  }
}
