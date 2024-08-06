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
    foreground = NSColor(red: 0, green: 0, blue: 0, alpha: 1)
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
