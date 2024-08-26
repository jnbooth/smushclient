import AppKit

extension RgbColor {
  func isBlack() -> Bool {
    self.r == 0 && self.g == 0 && self.b == 0
  }
}

struct OutputFormatter {
  private let echoCommands: Bool
  private let echoNewline: Bool
  private let fonts: OutputFonts
  private let echoAttributes: [NSAttributedString.Key: Any]
  let plainAttributes: [NSAttributedString.Key: Any]

  init() {
    fonts = OutputFonts()
    echoCommands = true
    echoNewline = true
    plainAttributes = [.font: fonts.provide()]
    echoAttributes = [.font: fonts.provide()]
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
  }
  
  func formatInput(_ input: String) -> NSAttributedString? {
    if !echoCommands {
      return nil
    }
    return NSAttributedString(string: echoNewline ? "\n" + input : input, attributes: echoAttributes)
  }

  func formatOutput(_ fragment: RustTextFragment) -> NSAttributedString {
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
