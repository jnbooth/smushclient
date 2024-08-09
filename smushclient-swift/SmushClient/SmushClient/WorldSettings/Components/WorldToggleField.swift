import SwiftUI

struct WorldToggleField: View {
  private var label: String
  private var value: Binding<Bool>

  init(_ label: String, _ value: Binding<Bool>) {
    self.label = label
    self.value = value
  }

  var body: some View {
    Toggle(isOn: value) {
      Text(label)
    }
    .toggleStyle(.checkbox)
  }
}

#Preview {
  WorldToggleField("Enable", .constant(false))
}
