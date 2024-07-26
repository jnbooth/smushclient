import Foundation

struct Key {
  static let FirstTime = "firstTime"
}

public class AppDefaults {
  let defaults = UserDefaults()

  var firstTime: Bool {
    get {
      !defaults.bool(forKey: Key.FirstTime)
    }
    set {
      defaults.set(!newValue, forKey: Key.FirstTime)
    }
  }
}
