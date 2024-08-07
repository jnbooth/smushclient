import AppKit

struct OutputFonts {
  private let base: NSFont
  private let bold: NSFont
  private let italic: NSFont
  private let boldAndItalic: NSFont

  init() {
    base = WorldModel.defaultFont
    bold = WorldModel.defaultFont
    italic = WorldModel.defaultFont
    boldAndItalic = WorldModel.defaultFont
  }

  init(_ world: WorldModel, fontManager: NSFontManager = NSFontManager.shared) {
    base = world.use_default_output_font ? WorldModel.defaultFont : world.output_font
    bold = world.show_bold ? fontManager.convert(base, toHaveTrait: .boldFontMask) : base

    if world.show_italic {
      italic = fontManager.convert(base, toHaveTrait: .italicFontMask)
      boldAndItalic = fontManager.convert(bold, toHaveTrait: .italicFontMask)
    } else {
      italic = base
      boldAndItalic = bold
    }
  }

  func provide(bold isBold: Bool = false, italic isItalic: Bool = false) -> NSFont {
    if isBold && isItalic {
      return boldAndItalic
    }
    if isBold {
      return bold
    }
    if isItalic {
      return italic
    }
    return base
  }
}
