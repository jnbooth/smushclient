import SwiftUI

struct WorldTextField: View {
  private var label: String
  private var value: Binding<String>

  init(_ label: String, _ value: Binding<String>) {
    self.label = label
    self.value = value
  }

  var body: some View {
    LabeledContent(label) {
      TextField("", text: value).frame(width: 250)
    }
  }
}

#Preview {
  var val = "Value"
  return WorldTextField("Name", Binding(get: { val }, set: { v in val = v }))
}