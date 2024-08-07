import SwiftUI

class WorldSettingsHostingController: NSHostingController<WorldSettingsView> {
  static weak var targetWorld: WorldModel?

  required init?(coder: NSCoder) {
    super.init(coder: coder, rootView: WorldSettingsView(world: Self.targetWorld!))
  }
}
