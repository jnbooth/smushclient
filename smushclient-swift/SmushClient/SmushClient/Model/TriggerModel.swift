import SwiftUI

@Observable
class TriggerModel {
  var reaction: ReactionModel = ReactionModel()
  var change_foreground: Bool = false
  var foreground: String = ""
  var foreground_color: NSColor? = nil
  var change_background: Bool = false
  var background: String = ""
  var background_color: NSColor? = nil
  var make_bold: Bool = false
  var make_italic: Bool = false
  var make_underline: Bool = false
  var sound: String = ""
  var sound_if_inactive: Bool = false
  var lowercase_wildcard: Bool = false
  var multi_line: Bool = false
  var lines_to_match: UInt8 = 0

  init() {
  }

  init(_ trigger: Trigger) {
    reaction = ReactionModel(trigger.reaction)
    change_foreground = trigger.change_foreground
    foreground = trigger.foreground.toString()
    foreground_color = NSColor(trigger.foreground_color)
    change_background = trigger.change_background
    background = trigger.background.toString()
    background_color = NSColor(trigger.background_color)
    make_bold = trigger.make_bold
    make_italic = trigger.make_italic
    make_underline = trigger.make_underline
    sound = trigger.sound.toString()
    sound_if_inactive = trigger.sound_if_inactive
    lowercase_wildcard = trigger.lowercase_wildcard
    multi_line = trigger.multi_line
    lines_to_match = trigger.lines_to_match
  }
}

extension Trigger {
  init(_ trigger: TriggerModel) {
    reaction = Reaction(trigger.reaction)
    change_foreground = trigger.change_foreground
    foreground = trigger.foreground.intoRustString()
    foreground_color = ColorOption(trigger.foreground_color)
    change_background = trigger.change_background
    background = trigger.background.intoRustString()
    background_color = ColorOption(trigger.background_color)
    make_bold = trigger.make_bold
    make_italic = trigger.make_italic
    make_underline = trigger.make_underline
    sound = trigger.sound.intoRustString()
    sound_if_inactive = trigger.sound_if_inactive
    lowercase_wildcard = trigger.lowercase_wildcard
    multi_line = trigger.multi_line
    lines_to_match = trigger.lines_to_match
  }
}
