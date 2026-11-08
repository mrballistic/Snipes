// Prints the CGWindowID of the first on-screen window owned by a process with
// the given name (default "Snipes"), for use with `screencapture -l`.
//
//   swift scripts/window-id.swift [process-name]
import CoreGraphics
import Foundation

let owner = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "Snipes"
let windows = CGWindowListCopyWindowInfo([.optionOnScreenOnly, .excludeDesktopElements], kCGNullWindowID) as? [[String: Any]] ?? []
for w in windows {
    guard let name = w[kCGWindowOwnerName as String] as? String, name.caseInsensitiveCompare(owner) == .orderedSame,
          let layer = w[kCGWindowLayer as String] as? Int, layer == 0,
          let id = w[kCGWindowNumber as String] as? Int else { continue }
    print(id)
    exit(0)
}
FileHandle.standardError.write("No on-screen window owned by \(owner)\n".data(using: .utf8)!)
exit(1)
