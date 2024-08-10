import SwiftUI

struct WorldRequiredTextField: View {
  private var label: String
  private var value: Binding<String>
  @State private var initialValue: String
  @FocusState private var focused: Bool

  init(_ label: String, _ value: Binding<String>) {
    self.label = label
    self.value = value
    self.initialValue = value.wrappedValue
    self.focused = false
  }

  var body: some View {
    WorldTextField(label, value).focused($focused).onChange(of: focused) {
      if focused {
        return
      }
      if !value.wrappedValue.isEmpty {
        initialValue = value.wrappedValue
        return
      }
      if !initialValue.isEmpty {
        value.wrappedValue = initialValue
      }
    }
  }
}

#Preview {
  WorldRequiredTextField("Name", .constant("Value"))
}
