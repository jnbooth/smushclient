import SwiftUI

struct WorldAddressView: View {
  @Bindable var world: WorldModel

  var body: some View {
    Form {
      WorldTextField("World Name", $world.name)
      WorldTextField("World Address", $world.site)
      WorldPortField("World Port", $world.port)
      Picker("Proxy Type", selection: $world.proxy_type) {
        Text("None").tag(ProxyType.None)
        Text("Socks4").tag(ProxyType.Socks4)
        Text("Socks5").tag(ProxyType.Socks5)
      }.frame(width: 175)
      WorldTextField("Proxy Address", $world.proxy_server)
      WorldPortField("Proxy Port", $world.proxy_port)
      WorldTextField("Proxy Username", $world.proxy_username)
      WorldSecureField("Proxy Password", $world.proxy_password)
      WorldToggleField("Save world automatically", $world.save_world_automatically)
    }
  }
}

#Preview {
  WorldAddressView(world: WorldModel())
}
