import Foundation

struct DefaultsKey {
  static let FirstTime = "firstTime"
}

public class AppDefaults {
  let defaults = UserDefaults()

  var firstTime: Bool {
    get {
      !defaults.bool(forKey: DefaultsKey.FirstTime)
    }
    set {
      defaults.set(!newValue, forKey: DefaultsKey.FirstTime)
    }
  }
}
