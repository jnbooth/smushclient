import Foundation

let homeDirURL = FileManager.default.homeDirectoryForCurrentUser.path()

struct RecentDocument: Identifiable {
  let id: String
  let name: String
  let dir: Substring
  let url: URL

  init(_ url: URL) {
    self.id = url.absoluteString
    self.name = url.lastPathComponent
    self.url = url
    let path = url.deletingLastPathComponent().path().dropLast(1)
    self.dir = path.starts(with: homeDirURL) ? "~/" + path[homeDirURL.endIndex...] : path
  }
}
