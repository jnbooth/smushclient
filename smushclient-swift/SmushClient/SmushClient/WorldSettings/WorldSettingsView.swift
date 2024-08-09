import SwiftUI

struct WorldSettingsView: View {
  @Environment(\.dismissWindow) private var dismissWindow
  @State private var world: WorldModel
  @State private var isNew: Bool
  
  init(world: WorldModel) {
    self.world = world
    self.isNew = world.site.isEmpty
  }

  var body: some View {
    WorldSettingsTabView(world: world)
    if isNew {
      HStack {
        Button("Cancel", role: .cancel) {
          world.site = ""
          dismissWindow()
        }
        Button("Connect") {
          isNew = false
          dismissWindow()
        }
      }.padding(.top, 2).padding(.bottom, 8)
    }
  }
}

#Preview {
  WorldSettingsView(world: WorldModel())
}
