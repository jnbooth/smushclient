import SwiftUI

let headingFont = Font.system(size: 20)

private struct BackgroundEffect: NSViewRepresentable {
  func makeNSView(context: Self.Context) -> NSView {
    let view = NSVisualEffectView()
    view.material = .underWindowBackground
    return view
  }

  func updateNSView(_ nsView: NSView, context: Context) {}
}

struct WelcomeView: View {
  @State var recentDocuments: [RecentDocument]

  var body: some View {
    VStack(alignment: .leading) {
      Text("Start").font(headingFont)
      Grid(alignment: .leading) {
        GridRow {
          Button(action: createWorld) {
            Image(systemName: "doc.badge.plus")
          }
          Button("New World", action: createWorld)
        }
        GridRow {
          Button(action: openWorld) {
            Image(systemName: "folder")
          }
          Button("Open World", action: openWorld)
        }
      }
      Text("Recent").font(headingFont).padding(.top, 10)
      Grid(alignment: .leading) {
        ForEach(recentDocuments) { doc in
          GridRow {
            Link(doc.name, destination: doc.url)
            Text(doc.dir).foregroundStyle(.gray)
          }
        }
      }
    }
    .buttonStyle(.link)
    .padding(.vertical, 50).padding(.horizontal, 100)
    .fixedSize()
    .frame(maxWidth: .infinity, maxHeight: .infinity)
    .background(BackgroundEffect())
    .environment(\.openURL, OpenURLAction(handler: openURL))
  }

  @MainActor private func createWorld() {
    NSDocumentController.shared.newDocument(self)
  }

  @MainActor private func openWorld() {
    NSDocumentController.shared.openDocument(self)
  }

  @MainActor private func openURL(_ url: URL) -> OpenURLAction.Result {
    NSDocumentController.shared.openDocument(withContentsOf: url, display: true) { doc, _, error in
      guard let error = error else {
        return
      }
      print(error)
      if doc != nil {
        return
      }
      removeRecent(url)
      NSAlert(error: error).runModal()
    }
    return .handled
  }

  @MainActor private func removeRecent(_ url: URL) {
    recentDocuments.removeAll { $0.url == url }
    let docs = NSDocumentController.shared
    docs.clearRecentDocuments(self)
    for recent in recentDocuments {
      docs.noteNewRecentDocumentURL(recent.url)
    }
  }
}

#Preview {
  WelcomeView(
    recentDocuments: [
      "/Users/j/Downloads/World1.smush",
      "/Users/j/Downloads/World2.smush",
      "/Users/j/Downloads/World3.smush",
    ].map { filename in RecentDocument(URL(filePath: filename, directoryHint: .notDirectory)) })
}
