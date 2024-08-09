import SwiftUI

struct WorldSettingsTabView: View {
  @State var world: WorldModel

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
    }
  }

#Preview {
  WorldSettingsTabView(world: WorldModel())
}
