import SwiftUI

struct WorldAddressView: View {
  @State private var world: WorldModel

  init(world: WorldModel) {
    self.world = world
  }

  var body: some View {
    Form {
      LabeledContent("World Name") {
        TextField("", text: $world.name).frame(width: 250)
      }
      LabeledContent("World IP/TCP Address") {
        TextField("", text: $world.site).frame(width: 250)
      }
      LabeledContent("World Port") {
        TextField("", value: $world.port, formatter: UInt16Formatter()).frame(maxWidth: 75)
      }
      Picker("Proxy Type", selection: $world.proxy_type) {
        Text("None").tag(ProxyType.None)
        Text("Socks4").tag(ProxyType.Socks4)
        Text("Socks5").tag(ProxyType.Socks5)
      }.frame(width: 175)
      LabeledContent("Proxy Address") {
        TextField("", text: $world.proxy_server).frame(width: 250)
      }
      LabeledContent("Proxy Port") {
        TextField("", value: $world.proxy_port, formatter: UInt16Formatter()).frame(maxWidth: 75)
      }
      LabeledContent("Proxy Username") {
        TextField("", text: $world.proxy_username).frame(width: 250)
      }
      LabeledContent("Proxy Password") {
        SecureField("", text: $world.proxy_password).frame(width: 250)
      }
      Toggle(isOn: $world.save_world_automatically) {
        Text("Save world automatically")
      }
      .toggleStyle(.checkbox)
    }.padding(.vertical, 10).padding(.horizontal, 100)
  }
}

#Preview {
  WorldAddressView(world: WorldModel())
}
