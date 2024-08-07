import AppKit

func optionalRustString(_ string: RustString) -> String? {
  let str = string.as_str()
  if str.len == 0 {
    return nil
  }
  return str.toString()
}

func fromVec<T, U>(_ vec: RustVec<T>, by: (_ item: T.SelfRef) -> U) -> [U] {
  let len = vec.len()
  var list: [U] = []
  list.reserveCapacity(len)
  for i in 0..<UInt(len) {
    list.append(by(vec.get(index: i)!))
  }
  return list
}

func intoVec<T, U>(_ list: [U], by: (_ item: U) -> T) -> RustVec<T> {
  let vec: RustVec<T> = RustVec()
  for item in list {
    vec.push(value: by(item))
  }
  return vec
}

func mappingFromRust(_ mapping: RustVec<KeypadMapping>) -> [String: String] {
  var dict: [String: String] = [:]
  dict.reserveCapacity(mapping.len())
  while let item = mapping.pop() {
    dict[item.keypad.toString()] = item.command.toString()
  }
  return dict
}

func mappingToRust(_ dict: [String: String]) -> RustVec<KeypadMapping> {
  let vec: RustVec<KeypadMapping> = RustVec()
  for (k, v) in dict {
    vec.push(value: KeypadMapping(keypad: k.intoRustString(), command: v.intoRustString()))
  }
  return vec
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
