import Foundation

struct DefaultsKey {
  static let FirstTime = "firstTime"
}

class AppDefaults {
  private let defaults = UserDefaults()

  var firstTime: Bool {
    get {
      !defaults.bool(forKey: DefaultsKey.FirstTime)
    }
    set {
      defaults.set(!newValue, forKey: DefaultsKey.FirstTime)
    }
  }
}
