# Building Snipes

This covers building the game from source, packaging the signed macOS app, and working on the code. For what the game is and how to play it, see the [README](README.md).

## Repository layout

| Path | What's there |
|---|---|
| `src/` | The game (`Snipes.cpp`) and its platform layers: `src/sdl/` for the SDL build (macOS, Linux, Windows) and `src/windows/` for the Windows console build |
| `src/config-sample.h` | Build options; copied to `src/config.h` on first build |
| `tests/` | Unit tests for the platform-independent logic |
| `fonts/` | IBM Plex Mono, used by the start screen and status bar (SIL Open Font License, see `fonts/IBMPlexMono-LICENSE.txt`) |
| `art/` | The app icon (`Icon.icon`, an Icon Composer file) and its source artwork |
| `scripts/` | Packaging and screenshot scripts |
| `docs/` | Screenshots, and the design and plan documents for the start screen and status bar |

## Dependencies

SDL builds need the SDL2 and SDL2_ttf libraries. On macOS with Homebrew:

```
brew install sdl2 sdl3 sdl2_ttf
```

(Homebrew’s `sdl2` is sdl2-compat, which runs on SDL3.)

On startup, the SDL build tries to load a custom font, `SnipesConsole.ttf`, which you can download separately from [here](http://kingbird.myphotos.cc/ee22d44076adb8a34d8e20df4be3730a/SnipesConsole.ttf). Put it in the repository root. Without it, the game falls back to a system monospace font (Menlo, or DejaVu Sans Mono on Linux), which won’t look quite like the original.

## With GNU Make

1. (Optional) Copy `src/config-sample.h` to `src/config.h`, and edit as desired.
2. Run `make` to compile an SDL build, `./snipes`.

Run it from the repository root so it finds `SnipesConsole.ttf` and `fonts/`.

For Arch Linux, you can use the [snipes-git](https://aur.archlinux.org/packages/snipes-git/) AUR package.

## With Visual Studio

1. Copy `src/config-sample.h` to `src/config.h` (and edit as/if desired).
2. Open and build the Visual Studio solution file (`Snipes.sln`).
   The project has configurations targeting SDL and Windows console graphics. The console build keeps the original text title screen and status line.

## The macOS app bundle

This is how the release build is made.

1. Install the dependencies above, and put `SnipesConsole.ttf` in the repository root so it gets bundled.
2. Run `scripts/make-macos-app.sh`. It builds the game, copies the Homebrew libraries, fonts and icon into the app, signs everything with your first “Developer ID Application” identity, and writes `dist/Snipes.app` plus a zip of it.

With `create-dmg` installed (`brew install create-dmg`), it also writes a disk image: the app on the left, an Applications shortcut on the right, and a background with an arrow between them, drawn by `scripts/dmg-background.swift`. The disk image is signed, and notarized along with the app.

To notarize as well, store your Apple credentials once, then pass the profile name:

```
xcrun notarytool store-credentials snipes-notary --apple-id <apple-id> --team-id <team-id>
NOTARY_PROFILE=snipes-notary scripts/make-macos-app.sh
```

The script header lists its other options, such as `SIGN_IDENTITY` and `BUNDLE_ID`.

The app is Apple Silicon only, because it bundles Homebrew’s arm64 libraries.

## Screenshots

To refresh `docs/screenshots/`, build the app, then run `scripts/capture-screenshots.sh`. It launches the app and types into it for about a minute, so leave the keyboard alone while it runs. It only types while the game is the frontmost app, and it restores your remembered start-screen level afterwards.

`SNIPES_NO_AUTOPAUSE=1` keeps the game running when its window isn’t in front (normally it pauses). The script sets it so the game keeps playing while it waits.

## Tests

`make test` builds and runs the unit tests (level descriptions, start-screen keys, the saved level, window fitting and time formatting), then checks that every source file with non-ASCII text starts with a UTF-8 byte order mark, which Visual Studio needs to read it correctly.

## What this fork changes under the hood

- The macOS SDL build runs the window and event loop on the main thread, as macOS requires, with the game logic on a worker thread.
- The window is high-DPI, so text and graphics render at full Retina resolution.
- The start screen and status bar are drawn by the SDL renderer (`src/sdl/startscreen.cpp`, `src/sdl/hud.cpp`). The game logic still writes the original status line; the SDL build just doesn’t draw those rows.
- The game falls back to a system font if `SnipesConsole.ttf` can’t be opened.
