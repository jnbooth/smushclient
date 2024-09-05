import SwiftUI

@Observable
class TriggerModel {
  var reaction: ReactionModel = ReactionModel()
  var change_foreground: Bool = false
  var foreground_color: NSColor = NSColor(srgbRed: 1, green: 1, blue: 1, alpha: 1)
  var change_background: Bool = false
  var background_color: NSColor = NSColor(srgbRed: 0, green: 0, blue: 0, alpha: 1)
  var make_bold: Bool = false
  var make_italic: Bool = false
  var make_underline: Bool = false
  var sound: String = ""
  var sound_if_inactive: Bool = false
  var lowercase_wildcard: Bool = false
  var multi_line: Bool = false
  var lines_to_match: UInt8 = 0

  init() {}

  init(_ trigger: Trigger) {
    reaction = ReactionModel(trigger.reaction)
    change_foreground = trigger.change_foreground
    foreground_color = NSColor(trigger.foreground_color)
    change_background = trigger.change_background
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
    foreground_color = RgbColor(trigger.foreground_color)
    change_background = trigger.change_background
    background_color = RgbColor(trigger.background_color)
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
