import AppKit

func optionalRustString(_ string: RustString) -> String? {
  let str = string.as_str()
  if str.len == 0 {
    return nil
  }
  return str.toString()
}

func fromVec<T, U>(_ vec: RustVec<T>, by: (_ item: T.SelfRef) -> U) -> [U] {
  let len = vec.len()
  var list: [U] = []
  list.reserveCapacity(len)
  for i in 0..<UInt(len) {
    list.append(by(vec.get(index: i)!))
  }
  return list
}

func intoVec<T, U>(_ list: [U], by: (_ item: U) -> T) -> RustVec<T> {
  let vec: RustVec<T> = RustVec()
  for item in list {
    vec.push(value: by(item))
  }
  return vec
}

func mappingFromRust(_ mapping: RustVec<KeypadMapping>) -> [String: String] {
  var dict: [String: String] = [:]
  dict.reserveCapacity(mapping.len())
  while let item = mapping.pop() {
    dict[item.keypad.toString()] = item.command.toString()
  }
  return dict
}

func mappingToRust(_ dict: [String: String]) -> RustVec<KeypadMapping> {
  let vec: RustVec<KeypadMapping> = RustVec()
  for (k, v) in dict {
    vec.push(value: KeypadMapping(keypad: k.intoRustString(), command: v.intoRustString()))
  }
  return vec
}
