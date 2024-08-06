import SwiftUI

struct WorldSettingsView: View {
  @Environment(\.dismissWindow) private var dismissWindow
  @State var world: WorldModel

  init(world: WorldModel) {
    self.world = world
  }

  var body: some View {
    TabView {
      TabView {
        WorldAddressView(world: world).tabItem {
          Text("Address")
        }
      }.tabItem {
        Label(String("Connecting"), systemImage: "globe")
      }
      TabView {

      }.tabItem {
        Label("Output", systemImage: "note.text")
      }
      TabView {

      }.tabItem {
        Label("Appearance", systemImage: "paintpalette")
      }
      TabView {

      }.tabItem {
        Label("Input", systemImage: "text.bubble")
      }
      TabView {

      }.tabItem {
        Label("Shortcuts", systemImage: "keyboard")
      }
    }.fixedSize()
    HStack {
      Button("Cancel", role: .cancel) {
        dismissWindow()
      }
      Button("Connect") {
        let notification = Notification(name: NotificationName.NewWorld, object: world)
        NotificationCenter.default.post(notification)
        dismissWindow()
      }
    }.padding(.top, 2).padding(.bottom, 8)
  }
}

#Preview {
  WorldSettingsView(world: WorldModel())
}
