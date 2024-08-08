import SwiftUI

private func PortFormatter() -> NumberFormatter {
  let formatter = NumberFormatter()
  formatter.minimum = 1
  formatter.maximum = NSNumber(value: UInt16.max)
  formatter.zeroSymbol = ""
  return formatter
}

struct WorldPortField: View {
  private static let formatter: NumberFormatter = PortFormatter()
  private var label: String
  private var value: Binding<UInt16>

  init(_ label: String, _ value: Binding<UInt16>) {
    self.label = label
    self.value = value
  }

  var body: some View {
    LabeledContent(label) {
      TextField("", value: value, formatter: Self.formatter).frame(maxWidth: 75)
    }
  }
}

#Preview {
  var val = UInt16(65535)
  return WorldPortField("Port", Binding(get: { val }, set: { v in val = v }))
}
