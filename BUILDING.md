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

Releases are universal apps (Apple Silicon and Intel) that run on macOS 15 and later. GitHub builds them; you sign and notarize them on your Mac.

### 1. Build on GitHub

Every push to `master` runs the **macOS universal build** workflow (`.github/workflows/macos-universal.yml`); you can also start it by hand from the Actions tab. It builds the app on Apple Silicon and Intel macOS 15 runners, using Homebrew’s macOS 15 libraries, merges the two with `scripts/merge-universal.sh`, and uploads an unsigned `Snipes-universal-unsigned` artifact.

### 2. Sign, notarize and package locally

Store your Apple notarization credentials once:

```
xcrun notarytool store-credentials snipes-notary --apple-id <apple-id> --team-id <team-id>
```

Then download the latest artifact and package it:

```
rm -rf dist/ci && gh run download --repo mrballistic/Snipes --name Snipes-universal-unsigned --dir dist/ci
ditto -x -k dist/ci/Snipes-universal-unsigned.zip dist/ci
NOTARY_PROFILE=snipes-notary scripts/make-macos-app.sh --app dist/ci/Snipes.app
```

That signs everything with your first “Developer ID Application” identity, notarizes and staples it, and writes `dist/Snipes-<version>-macos-universal.zip`. With `create-dmg` installed (`brew install create-dmg`), it also writes a matching `.dmg`: the app on the left, an Applications shortcut on the right, and a background with an arrow between them, drawn by `scripts/dmg-background.swift`. The disk image is signed and notarized too.

### Building just for your own Mac

`scripts/make-macos-app.sh` with no arguments builds, signs and packages an app for your Mac’s architecture only, from your local Homebrew libraries. Put `SnipesConsole.ttf` in the repository root first so it gets bundled. The app’s minimum macOS comes from what its bundled libraries need, which for a local Homebrew is your current macOS, so use the GitHub build for anything you share. `--no-sign` stops after building, which is what CI uses.

The script header lists its other options, such as `SIGN_IDENTITY` and `BUNDLE_ID`.

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
