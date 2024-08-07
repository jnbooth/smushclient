import AppKit

struct InputFormatter {
  private let display: Bool
  private let noNewline: Bool
  private let attributes: [NSAttributedString.Key: Any]

  init() {
    display = true
    noNewline = false
    attributes = [:]
  }

  init(_ world: WorldModel) {
    display = world.display_my_input
    noNewline = world.keep_commands_on_same_line
    let font = world.use_default_input_font ? WorldModel.defaultFont : world.input_font

    var attrs: [NSAttributedString.Key: Any] = [.font: font]

    if let foreground = world.input_colors.foreground {
      attrs[.foregroundColor] = foreground
    }

    if let background = world.input_colors.background {
      attrs[.backgroundColor] = background
    }

    attributes = attrs
  }

  func format(_ input: String) -> NSAttributedString? {
    if !display {
      return nil
    }

    return NSAttributedString(string: noNewline ? input : "\n" + input, attributes: attributes)
  }
}
