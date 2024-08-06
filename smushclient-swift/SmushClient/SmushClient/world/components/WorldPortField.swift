import SwiftUI

func UInt16Formatter() -> NumberFormatter {
  let formatter = NumberFormatter()
  formatter.minimum = 0
  formatter.maximum = NSNumber(value: UInt16.max)
  return formatter
}

struct WorldPortField: View {
  var label: String
  var value: Binding<UInt16>

  init(_ label: String, _ value: Binding<UInt16>) {
    self.label = label
    self.value = value
  }

  var body: some View {
    LabeledContent(label) {
      TextField("", value: value, formatter: UInt16Formatter()).frame(maxWidth: 75)
    }
  }
}

#Preview {
  var val = UInt16(65535)
  return WorldPortField("Port", Binding(get: { val }, set: { v in val = v }))
}
