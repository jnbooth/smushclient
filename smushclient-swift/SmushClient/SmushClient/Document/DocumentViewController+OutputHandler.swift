import AppKit
import SwiftUI

private let hrString = "\n\r\u{00A0} \u{0009} \u{00A0}\n\n"

func lastLineRange(_ string: NSString) -> NSRange {
  var i = string.length
  while i > 0 {
    i -= 1
    if string.character(at: i) == 10 {
      break
    }
  }
  let precededByCR = i > 0 && string.character(at: i - 1) == 13
  if precededByCR {
    i -= 1
  }
  return NSRange(location: i, length: string.length - i)
}

extension DocumentViewController {
  func handleOutput(_ output: RustOutputStream) {
    if output.count() == 0 {
      return
    }

    textStorage.beginEditing()
    var shouldScrollToBottom = false
    defer {
      textStorage.endEditing()
      if shouldScrollToBottom {
        scrollToBottom()
      }
    }

    while let fragment = output.next() {
      if handleFragment(fragment) {
        shouldScrollToBottom = true
      }
    }
  }

  private func handleFragment(_ fragment: OutputFragment) -> Bool {
    switch fragment {
    case .Effect(let effect):
      handleEffect(effect)
    case .Hr:
      handleHR()
    case .LineBreak:
      handleLineBreak()
    case .MxpError(let error):
      handleMxpError(error.toString())
    case .MxpEntitySet(let name, let value, let publish, let is_variable):
      handleMxpVariable(
        name: name.toString(), value: value.toString(), publish: publish, is_variable: is_variable)
    case .MxpEntityUnset(let name, let is_variable):
      handleMxpVariable(name: name.toString(), value: nil, is_variable: is_variable)
    case .PageBreak:
      handlePageBreak()
    case .Send(let send):
      handleSend(send)
    case .Sound(let sound):
      handleSound(sound.toString())
    case .Telnet(let sequence):
      handleTelnet(sequence)
    case .Text(let text):
      handleText(text)
    }
  }

  private func handleEffect(_ effect: EffectFragment) -> Bool {
    switch effect {
    case .Backspace, .EraseCharacter:
      if willBreak == true {
        willBreak = false
        return true
      }
      let textString = textStorage.mutableString
      if textString.length == 0 {
        return false
      }
      let lastCharRange = textString.rangeOfComposedCharacterSequence(at: textString.length - 1)
      textStorage.deleteCharacters(in: lastCharRange)
      return false

    case .Beep:
      Task {
        let _ = await MainActor.run {
          NSApplication.shared.requestUserAttention(.criticalRequest)
        }
      }
      return false

    case .CarriageReturn:
      textStorage.append(NSAttributedString("\r"))
      return true

    case .EraseLine:
      let textString = textStorage.mutableString
      if textString.length == 0 {
        return false
      }
      textStorage.deleteCharacters(in: lastLineRange(textString))
      willBreak = true
      return true
    }
  }

  private func handleHR() -> Bool {
    textStorage.append(
      NSAttributedString(
        string: hrString,
        attributes: [
          .strikethroughStyle: NSUnderlineStyle.single.rawValue,
          .strikethroughColor: NSColor.gray,
        ]))
    willBreak = false
    return true
  }

  private func handleImage(_ src: String) -> Bool {
    var attributes = formatter.plainAttributes
    attributes[.link] = src
    textStorage.append(NSAttributedString(string: src, attributes: attributes))
    return true
  }

  private func handleLineBreak() -> Bool {
    let didBreak = handleBreak()
    willBreak = true
    return didBreak
  }

  private func handleMxpError(_ error: String) -> Bool {
    print(error)
    return false
  }

  private func handleMxpVariable(
    name: String, value: String?, publish: Bool = false, is_variable: Bool = false
  ) -> Bool {
    return false
  }

  private func handlePageBreak() -> Bool {
    handleLineBreak()
  }

  private func handleSound(_ location: String) -> Bool {
    false
  }

  private func handleTelnet(_ sequence: TelnetFragment) -> Bool {
    false
  }

  private func handleText(_ text: RustTextFragment) -> Bool {
    let _ = handleBreak()
    textStorage.append(formatter.formatOutput(text))
    return true
  }

  private func handleBreak() -> Bool {
    if !willBreak {
      return false
    }
    let attributes = formatter.plainAttributes
    textStorage.append(NSAttributedString(string: "\n", attributes: attributes))
    willBreak = false
    return true
  }
}
