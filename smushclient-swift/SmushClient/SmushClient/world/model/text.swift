import SwiftUI

extension HexColor {
  init(_ color: NSColor) {
    self.r = UInt8(color.redComponent * 255)
    self.g = UInt8(color.greenComponent * 255)
    self.b = UInt8(color.blueComponent * 255)
  }
}

extension ColorOption {
  init(_ color: NSColor?) {
    if let color = color {
      self = .Some(HexColor(color))
    } else {
      self = .None
    }
  }
}

extension NSColor {
  convenience init(_ color: HexColor) {
    self.init(
      red: CGFloat(color.r) / 255,
      green: CGFloat(color.g) / 255,
      blue: CGFloat(color.b) / 255,
      alpha: 1)
  }

  convenience init?(_ color: ColorOption) {
    switch color {
    case .None:
      return nil
    case .Some(let color):
      self.init(color)
    }
  }
}

@Observable
class ColorPairModel {
  var foreground: NSColor?
  var background: NSColor?

  init() {
    foreground = NSColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1)
    background = nil
  }

  init(foreground: NSColor? = nil, background: NSColor? = nil) {
    self.foreground = foreground
    self.background = background
  }

  init(_ pair: ColorPair) {
    foreground = NSColor(pair.foreground)
    background = NSColor(pair.background)
  }
}

extension ColorPair {
  init(_ pair: ColorPairModel) {
    foreground = ColorOption(pair.foreground)
    background = ColorOption(pair.background)
  }
}

struct NSColorPair {
  let foreground: NSColor?
  let background: NSColor?
  
  init(foreground: NSColor? = nil, background: NSColor? = nil) {
    self.foreground = foreground
    self.background = background
  }
  
  init(_ pair: ColorPairModel) {
    foreground = pair.foreground
    background = pair.background
  }
}

extension NSFont {
 convenience init?(_ rustString: RustString, manager: NSFontManager = NSFontManager.shared) {
    guard
      let fontString = optionalRustString(rustString),
      let delimIndex = fontString.lastIndex(of: "-"),
      let size = Double(fontString[fontString.index(after: delimIndex)...])
    else {
      return nil
    }
    self.init(name: String(fontString[..<delimIndex]), size: CGFloat(size))
  }
}

extension NSFont: IntoRustString {
  public func intoRustString() -> RustString {
    RustString(String(format: "%@-%f", fontName, pointSize))
  }
}
