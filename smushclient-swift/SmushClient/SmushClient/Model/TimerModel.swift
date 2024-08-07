import SwiftUI

@Observable
class TimerModel {
  var send: SenderModel = SenderModel()
  var occurrence: Occurrence = .Interval(s: 0)
  var active_closed: Bool = false

  init() {}

  init(_ timer: Timer) {
    send = SenderModel(timer.send)
    occurrence = timer.occurrence
    active_closed = timer.active_closed
  }
}

extension Timer {
  init(_ timer: TimerModel) {
    send = Sender(timer.send)
    occurrence = timer.occurrence
    active_closed = timer.active_closed
  }
}
