import AppKit

extension RgbColor {
  func isBlack() -> Bool {
    self.r == 0 && self.g == 0 && self.b == 0
  }
}

private typealias Attributes = [NSAttributedString.Key: Any]

private func createNamedColors() -> [RgbColor: NSColor] {
  var colors: [RgbColor: NSColor] = Dictionary(minimumCapacity: 166)
  let iter = RustNamedColorIter()
  while let color = iter.next() {
    colors[color] = NSColor(color)
  }
  return colors
}

struct OutputFormatter {
  private let echoCommands: Bool
  private let echoNewline: Bool
  private let fonts: OutputFonts
  private let echoAttributes: Attributes
  private let palette: Palette
  let plainAttributes: [NSAttributedString.Key: Any]

  init() {
    fonts = OutputFonts()
    echoCommands = true
    echoNewline = true
    plainAttributes = [.font: fonts.provide()]
    echoAttributes = [.font: fonts.provide()]
    palette = Palette()
  }

  init(_ world: WorldModel) {
    fonts = OutputFonts(world)
    echoCommands = world.display_my_input
    echoNewline = !world.keep_commands_on_same_line
    let baseFont = fonts.provide()
    plainAttributes = [.font: baseFont]
    var echoAttrs: [NSAttributedString.Key: Any] = [.font: baseFont]
    if let foreground = world.echo_colors.foreground {
      echoAttrs[.foregroundColor] = foreground
    }
    if let background = world.echo_colors.background {
      echoAttrs[.backgroundColor] = background
    }
    echoAttributes = echoAttrs
    var paletteMut = Palette.namedColors
    paletteMut.insert(world.note_text_colour)
    paletteMut.insert(world.error_colour)
    for color in world.ansi_colors {
      paletteMut.insert(color)
    }
    palette = paletteMut
  }

  func formatInput(_ input: String) -> NSAttributedString? {
    if !echoCommands {
      return nil
    }
    return NSAttributedString(
      string: echoNewline ? "\n" + input : input, attributes: echoAttributes)
  }

  func formatOutput(_ fragment: RustTextFragment) -> NSAttributedString {
    let text = fragment.text().toString()

    var attrs: [NSAttributedString.Key: Any] = [
      .font: fonts.provide(bold: fragment.isBold(), italic: fragment.isItalic()),
      .foregroundColor: palette[fragment.foreground()],
    ]

    let background = fragment.background()
    if !background.isBlack() {
      attrs[.backgroundColor] = palette[background]
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
