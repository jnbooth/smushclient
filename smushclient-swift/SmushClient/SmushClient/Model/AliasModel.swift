import SwiftUI

@Observable
class AliasModel {
  var reaction: ReactionModel = ReactionModel()
  var echo_alias: Bool = false
  var menu: Bool = false
  var omit_from_command_history: Bool = false

  init() {

  }

  init(_ alias: Alias) {
    reaction = ReactionModel(alias.reaction)
    echo_alias = alias.echo_alias
    menu = alias.menu
    omit_from_command_history = alias.omit_from_command_history
  }
}

extension Alias {
  init(_ alias: AliasModel) {
    reaction = Reaction(alias.reaction)
    echo_alias = alias.echo_alias
    menu = alias.menu
    omit_from_command_history = alias.omit_from_command_history
  }
}
