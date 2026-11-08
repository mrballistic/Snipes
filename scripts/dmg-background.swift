// Draws the DMG window background: light, so Finder's black icon labels stay
// readable, with a blue arrow from the app to the Applications folder and a
// one-line hint in the start screen's typeface.
//
//   swift scripts/dmg-background.swift <output.png> <scale>
//
// scripts/make-macos-app.sh renders it at 1x and 2x and combines them into a
// HiDPI TIFF. Icon positions must match the create-dmg call in that script.
import AppKit
import CoreText

let args = CommandLine.arguments
let output = args.count > 1 ? args[1] : "dmg-background.png"
let scale = args.count > 2 ? CGFloat(Double(args[2]) ?? 1) : 1

// Window content size and icon centers, in points (Finder coordinates: origin top-left)
let width: CGFloat = 640, height: CGFloat = 400
let appX: CGFloat = 170, appsX: CGFloat = 470, iconY: CGFloat = 185

// IBM Plex Mono from fonts/, if present
let fontDir = URL(fileURLWithPath: CommandLine.arguments[0]).deletingLastPathComponent().appendingPathComponent("../fonts")
for name in ["IBMPlexMono-Regular.ttf", "IBMPlexMono-Bold.ttf"] {
    CTFontManagerRegisterFontsForURL(fontDir.appendingPathComponent(name) as CFURL, .process, nil)
}
func font(_ name: String, _ size: CGFloat) -> NSFont {
    NSFont(name: name, size: size * scale) ?? NSFont.monospacedSystemFont(ofSize: size * scale, weight: .regular)
}
func color(_ hex: UInt32, _ alpha: CGFloat = 1) -> NSColor {
    NSColor(srgbRed: CGFloat((hex >> 16) & 0xff) / 255, green: CGFloat((hex >> 8) & 0xff) / 255, blue: CGFloat(hex & 0xff) / 255, alpha: alpha)
}

let pixelsWide = Int(width * scale), pixelsHigh = Int(height * scale)
guard let rep = NSBitmapImageRep(bitmapDataPlanes: nil, pixelsWide: pixelsWide, pixelsHigh: pixelsHigh,
                                 bitsPerSample: 8, samplesPerPixel: 4, hasAlpha: true, isPlanar: false,
                                 colorSpaceName: .deviceRGB, bytesPerRow: 0, bitsPerPixel: 0) else { exit(1) }
NSGraphicsContext.saveGraphicsState()
NSGraphicsContext.current = NSGraphicsContext(bitmapImageRep: rep)

// AppKit's origin is bottom-left; convert from Finder's top-left coordinates
func y(_ finderY: CGFloat) -> CGFloat { (height - finderY) * scale }
func x(_ v: CGFloat) -> CGFloat { v * scale }

// Background: a subtle vertical gradient
NSGradient(starting: color(0xf6f8fc), ending: color(0xe4e9f2))!.draw(in: NSRect(x: 0, y: 0, width: x(width), height: y(0)), angle: -90)

// Arrow between the two icons, with a soft halo
let arrowStart = appX + 70, arrowEnd = appsX - 70
for (lineWidth, alpha) in [(CGFloat(8), CGFloat(0.12)), (3, 1.0)] {
    let path = NSBezierPath()
    path.lineWidth = lineWidth * scale
    path.lineCapStyle = .round
    path.lineJoinStyle = .round
    path.move(to: NSPoint(x: x(arrowStart), y: y(iconY)))
    path.line(to: NSPoint(x: x(arrowEnd), y: y(iconY)))
    path.move(to: NSPoint(x: x(arrowEnd - 14), y: y(iconY - 12)))
    path.line(to: NSPoint(x: x(arrowEnd), y: y(iconY)))
    path.line(to: NSPoint(x: x(arrowEnd - 14), y: y(iconY + 12)))
    color(0x1f7fd1, alpha).setStroke()
    path.stroke()
}

// Hint text, centered near the bottom
func drawCentered(_ text: String, _ f: NSFont, _ c: NSColor, finderY: CGFloat, tracking: CGFloat = 0) {
    let attrs: [NSAttributedString.Key: Any] = [.font: f, .foregroundColor: c, .kern: tracking * scale]
    let s = NSAttributedString(string: text, attributes: attrs)
    let size = s.size()
    s.draw(at: NSPoint(x: (x(width) - size.width) / 2, y: y(finderY) - size.height / 2))
}
drawCentered("Drag Snipes into Applications to install it", font("IBMPlexMono", 14), color(0x1d2740), finderY: 318)
drawCentered("SIGNED AND NOTARIZED · APPLE SILICON", font("IBMPlexMono", 10), color(0x5b6680), finderY: 348, tracking: 1.5)

NSGraphicsContext.restoreGraphicsState()
guard let png = rep.representation(using: .png, properties: [:]) else { exit(1) }
try! png.write(to: URL(fileURLWithPath: output))
