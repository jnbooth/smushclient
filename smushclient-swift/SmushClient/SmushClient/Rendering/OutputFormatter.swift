import AppKit

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

struct OutputFormatter {
  let fonts: OutputFonts
  let showUnderline: Bool
  let hyperlinkColor: NSColor?
  let underlineHyperlinks: Bool
  let ansiColors: AnsiColors

  init() {
    fonts = OutputFonts()
    showUnderline = true
    hyperlinkColor = nil
    underlineHyperlinks = true
    ansiColors = defaultAnsiColors
  }

  init(_ world: WorldModel) {
    fonts = OutputFonts(world)
    showUnderline = world.show_underline
    hyperlinkColor = world.use_custom_link_color ? world.hyperlink_color : nil
    underlineHyperlinks = world.underline_hyperlinks
    ansiColors = world.use_default_colors ? defaultAnsiColors : world.ansi_colors
  }

  func format(_ fragment: RustTextFragment) -> NSAttributedString {
    let text = fragment.text().toString()

    let invert = fragment.isInverse()
    let foreground = invert ? fragment.background() : fragment.foreground()
    let background = invert ? fragment.foreground() : fragment.background()

    var attrs: [NSAttributedString.Key: Any] = [
      .font: fonts.provide(bold: fragment.isBold(), italic: fragment.isItalic()),
      .foregroundColor: foreground.color(ansiColors),
    ]

    if !background.isBlack() {
      attrs[.backgroundColor] = background.color(ansiColors)
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
