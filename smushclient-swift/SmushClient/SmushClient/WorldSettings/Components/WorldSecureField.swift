import SwiftUI

struct WorldSecureField: View {
  private var label: String
  private var value: Binding<String>

  init(_ label: String, _ value: Binding<String>) {
    self.label = label
    self.value = value
  }

  var body: some View {
    LabeledContent(label) {
      SecureField("", text: value).frame(width: 250)
    }
  }
}

#Preview {
  var val = "Value"
  return WorldSecureField("Password", Binding(get: { val }, set: { v in val = v }))
}
