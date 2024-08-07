import AppKit

typealias AnsiColors = [NSColor]

let defaultAnsiColors: AnsiColors = [
  NSColor(red: 0, green: 0, blue: 0, alpha: 1),  // black
  NSColor(red: 0.5, green: 0, blue: 0, alpha: 1),  // red
  NSColor(red: 0, green: 0.5, blue: 0, alpha: 1),  // green
  NSColor(red: 0.5, green: 0.5, blue: 0, alpha: 1),  // yellow
  NSColor(red: 0, green: 0, blue: 0.5, alpha: 1),  // blue
  NSColor(red: 0.5, green: 0, blue: 0.5, alpha: 1),  // purple
  NSColor(red: 0, green: 0.5, blue: 0.5, alpha: 1),  // cyan
  NSColor(red: 0.75, green: 0.75, blue: 0.75, alpha: 1),  // white
  NSColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1),  // bright black
  NSColor(red: 1, green: 0, blue: 0, alpha: 1),  // bright red
  NSColor(red: 0, green: 1, blue: 0, alpha: 1),  // bright green
  NSColor(red: 1, green: 1, blue: 0, alpha: 1),  //bright yellow
  NSColor(red: 0, green: 0, blue: 1, alpha: 1),  // bright blue
  NSColor(red: 1, green: 0, blue: 1, alpha: 1),  // bright purple
  NSColor(red: 0, green: 1, blue: 1, alpha: 1),  // bright cyan
  NSColor(red: 1, green: 1, blue: 1, alpha: 1),  // bright white
]
