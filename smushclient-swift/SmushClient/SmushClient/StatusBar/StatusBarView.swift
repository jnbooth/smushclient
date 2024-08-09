import SwiftUI

private let connectedColor = Color(white: 0.8)
private let disconnectedColor = Color(white: 0.4)

struct StatusBarView: View {
  var status: StatusBarState

  var body: some View {
    HStack {
      Text(status.connected ? "Connected" : "Disconnected")
        .foregroundStyle(status.connected ? connectedColor : disconnectedColor)
      Text(status.pluginMessage)
    }.padding(.horizontal, 10).padding(.vertical, 5)
  }
}

#Preview {
  StatusBarView(status: StatusBarState())
}
