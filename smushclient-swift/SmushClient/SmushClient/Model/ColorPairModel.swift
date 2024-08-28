import SwiftUI

extension RgbColor: Equatable {
  public static func == (lhs: RgbColor, rhs: RgbColor) -> Bool {
    lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b
  }
}

extension RgbColor: Hashable {
  public func hash(into hasher: inout Hasher) {
    hasher.combine(r)
    hasher.combine(g)
    hasher.combine(g)
  }
}

extension RgbColor: @unchecked Sendable {}

@Observable
class ColorPairModel {
  var foreground: NSColor?
  var background: NSColor?

  init() {
    foreground = NSColor(srgbRed: 0.5, green: 0.5, blue: 0.5, alpha: 1)
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
