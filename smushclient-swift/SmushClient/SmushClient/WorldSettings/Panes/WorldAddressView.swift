import SwiftUI

struct WorldAddressView: View {
  @Bindable var world: WorldModel

  var body: some View {
    let proxyDisabled = world.proxy_type == .None
    Form {
      WorldRequiredTextField("World Name*", $world.name)
      WorldRequiredTextField("World Address*", $world.site)
      WorldPortField("World Port*", $world.port)
      Picker("Proxy Type", selection: $world.proxy_type) {
        Text("None").tag(ProxyType.None)
        Text("Socks4").tag(ProxyType.Socks4)
        Text("Socks5").tag(ProxyType.Socks5)
      }.frame(width: 175)
      WorldRequiredTextField(
        proxyDisabled ? "Proxy Address" : "Proxy Address*", $world.proxy_server
      ).disabled(proxyDisabled)
      WorldPortField(proxyDisabled ? "Proxy Port" : "Proxy Port*", $world.proxy_port).disabled(
        proxyDisabled)
      WorldTextField("Proxy Username", $world.proxy_username).disabled(proxyDisabled)
      WorldSecureField("Proxy Password", $world.proxy_password).disabled(proxyDisabled)
      WorldToggleField("Save world automatically", $world.save_world_automatically)
    }
  }
}

#Preview {
  WorldAddressView(world: WorldModel())
}
