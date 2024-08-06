import SwiftUI

@Observable
class ReactionModel {
  var sequence: Int16 = 100
  var pattern: String = ""
  var send: SenderModel = SenderModel()
  var ignore_case: Bool = false
  var keep_evaluating: Bool = false
  var is_regex: Bool = false
  var expand_variables: Bool = false
  var repeats: Bool = false
  var regex: String = "^$"

  init() {

  }

  init(_ reaction: Reaction) {
    sequence = reaction.sequence
    pattern = reaction.pattern.toString()
    send = SenderModel(reaction.send)
    ignore_case = reaction.ignore_case
    keep_evaluating = reaction.keep_evaluating
    is_regex = reaction.is_regex
    expand_variables = reaction.expand_variables
    repeats = reaction.repeats
    regex = reaction.regex.toString()
  }
}

extension Reaction {
  init(_ reaction: ReactionModel) {
    sequence = reaction.sequence
    pattern = reaction.pattern.intoRustString()
    send = Sender(reaction.send)
    ignore_case = reaction.ignore_case
    keep_evaluating = reaction.keep_evaluating
    is_regex = reaction.is_regex
    expand_variables = reaction.expand_variables
    repeats = reaction.repeats
    regex = reaction.regex.intoRustString()
  }
}
