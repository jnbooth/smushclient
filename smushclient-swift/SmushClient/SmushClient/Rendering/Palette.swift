import AppKit

struct Palette {
  static let namedColors: Self = createNamedColors()
  private var palette: [RgbColor: NSColor]

  init() {
    palette = Dictionary()
  }

  init(minimumCapacity: Int) {
    palette = Dictionary(minimumCapacity: minimumCapacity)
  }

  private static func createNamedColors() -> Self {
    var namedColors = Self(minimumCapacity: 166)
    let iter = RustNamedColorIter()
    while let color = iter.next() {
      namedColors.insert(color)
    }
    return namedColors
  }

  mutating func insert(_ color: RgbColor) {
    palette[color] = NSColor(color)
  }

  mutating func insert(_ color: NSColor) {
    palette[RgbColor(color)] = color
  }

  subscript(index: RgbColor) -> NSColor {
    palette[index] ?? NSColor(index)
  }

  func get(_ index: RgbColor) -> NSColor {
    self[index]
  }
  
}
