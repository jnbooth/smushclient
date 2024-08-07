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

extension MainViewController {
  func handleOutput(_ output: RustOutputStream) {
    textStorage.beginEditing()
    var shouldScrollToBottom = false
    while let fragment = output.next() {
      if handleFragment(fragment) {
        shouldScrollToBottom = true
      }
    }
    textStorage.endEditing()
    if shouldScrollToBottom {
      scrollToBottom()
    }
  }

  private func handleFragment(_ fragment: OutputFragment) -> Bool {
    switch fragment {
    case .Effect(let effect):
      handleEffect(effect)
    case .Hr:
      handleHR()
    case .Image(let src):
      handleImage(src.toString())
    case .LineBreak:
      handleLineBreak()
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
    var attributes = outputFormatter.plainAttributes
    attributes[.link] = src
    textStorage.append(NSAttributedString(string: src, attributes: attributes))
    return true
  }

  private func handleLineBreak() -> Bool {
    let didBreak = handleBreak()
    willBreak = true
    return didBreak
  }

  private func handlePageBreak() -> Bool {
    handleLineBreak()
  }

  private func handleSend(_ send: SendRequest) -> Bool {
    let text = send.text.toString()
    switch send.send_to {
    case .Command:
      setInput(text)

    case .Output:
      let attributes = outputFormatter.plainAttributes
      textStorage.append(NSAttributedString(string: text, attributes: attributes))
      return true

    case .Status:
      status.pluginMessage = text

    case .World, .WorldDelay, .WorldImmediate:
      sendInput(text)

    default:
      break
    }

    return false
  }

  private func handleSound(_ location: String) -> Bool {
    false
  }

  private func handleTelnet(_ sequence: TelnetFragment) -> Bool {
    false
  }

  private func handleText(_ text: RustTextFragment) -> Bool {
    let _ = handleBreak()
    textStorage.append(outputFormatter.format(text))
    return true
  }

  private func handleBreak() -> Bool {
    if !willBreak {
      return false
    }
    let attributes = outputFormatter.plainAttributes
    textStorage.append(NSAttributedString(string: "\n", attributes: attributes))
    willBreak = false
    return true
  }
}
