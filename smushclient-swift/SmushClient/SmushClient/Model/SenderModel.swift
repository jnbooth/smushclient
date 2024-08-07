import SwiftUI

@Observable
class SenderModel {
  var send_to: SendTarget = .World
  var label: String = ""
  var script: String = ""
  var group: String = ""
  var variable: String = ""
  var text: String = ""
  var enabled: Bool = true
  var one_shot: Bool = false
  var temporary: Bool = false
  var omit_from_output: Bool = false
  var omit_from_log: Bool = false

  init() {

  }

  init(_ sender: Sender) {
    send_to = sender.send_to
    label = sender.label.toString()
    script = sender.script.toString()
    group = sender.group.toString()
    variable = sender.variable.toString()
    text = sender.text.toString()
    enabled = sender.enabled
    one_shot = sender.one_shot
    temporary = sender.temporary
    omit_from_output = sender.omit_from_output
    omit_from_log = sender.omit_from_log
  }
}

extension Sender {
  init(_ sender: SenderModel) {
    send_to = sender.send_to
    label = sender.label.intoRustString()
    script = sender.script.intoRustString()
    group = sender.group.intoRustString()
    variable = sender.variable.intoRustString()
    text = sender.text.intoRustString()
    enabled = sender.enabled
    one_shot = sender.one_shot
    temporary = sender.temporary
    omit_from_output = sender.omit_from_output
    omit_from_log = sender.omit_from_log
  }
}
