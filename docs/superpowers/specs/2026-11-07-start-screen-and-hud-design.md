# Modern start screen and in-game bar — design

Date: 2026-11-07
Repo: `mrballistic/Snipes` (fork of `Davidebyzero/Snipes`), branch `macos-main-thread`, SDL build.

## Intent

Snipes currently opens on a plain white text screen in its 40×25 character grid: the SNIPES ASCII art, the copyright, and a bare prompt, `Enter skill level (A-Z)(1-9):`, which tells a new player nothing about what the letter and the number mean. The three-row status bar at the top of the game is similarly cryptic: cyan and white numbers with no labels, and icons that don't explain themselves.

The goal is to make the game approachable to someone opening the macOS app for the first time, without changing the game itself:

1. **A modern start screen** drawn with smooth graphics (anti-aliased UI text, not the character grid) that keeps the original SNIPES art, explains the controls, and makes level selection self-explanatory.
2. **A redesigned in-game bar** in the same visual language, showing only what the player wants at a glance, and taking up less vertical space.

Approved mockups (in `.superpowers/brainstorm/37227-1794110264/content/`, not committed): `start-screen-a-v2.html` for the start screen, `hud-final-v4.html` for the bar.

## Goals

- Replace the text start screen and its skill prompt in the SDL build with a graphical start screen.
- Make the difficulty system legible: what A–Z and 1–9 each control, and what the chosen level actually means.
- Replace the three-row status bar with a single slim graphical bar showing snipes left, portals left, time and score.
- Return to the start screen after a game when the player chooses to play again.

## Non-goals

- **No change to game logic, timing, or the replay file format.** Existing `.SnipesGame` replays must play back identically. The skill level is still a letter (0–25) and a number (1–9), stored exactly as now.
- **No change to the playfield.** It stays the character-grid rendering it is now, the same 22 rows × 40 columns, the same font and colors.
- **The Windows console build** (`windows/`, `_CONSOLE`) keeps its current text title screen and status bar. Only the SDL build changes. The Visual Studio SDL configuration should still compile.
- End-of-game messages ("The SNIPES have triumphed!!!", "Play another game? (Y or N)") stay as they are, in grid text at the bottom. Restyling them is a possible later step, not this one.

## Anti-goals (would make this a failure even if it works)

- Any change in gameplay or replay determinism.
- Showing *less* of the maze, or changing its proportions.
- Text that is blurry on a Retina display: "smooth graphics" is the point.
- A start screen that only works with a mouse. The game is keyboard-driven; the screen must be fully usable from the keyboard.

## The start screen (decided)

Direction "A · Two strips", revised. Dark background (near-black navy, `#05070d` in the mockup), cyan accent (`#7fe7ff`).

Top to bottom, all centered:

1. **The original SNIPES ASCII art**, the same 10 lines of ↑ and ☺ glyphs as today's title (`TITLE_SCREEN` in `Snipes.cpp`), drawn in `SnipesConsole.ttf` (the game font, with the existing fallback chain), in the cyan accent with a soft glow, CRT-phosphor style.
2. **"Enemy skill" strip:** the letters A–Z in a row; the selected one is a filled cyan tile. Labelled with its keys, ← →.
3. **"Enemy numbers" strip:** the digits 1–9, same treatment. Labelled ↑ ↓.
4. **Level card**, text centered: `Level M4 · 40 snipes · 4 portals · 5 lives`, and beneath it a centered row of four hazard tags, which are lit when that hazard is on at the chosen letter and dim otherwise: *Small snipes*, *Bouncing bullets*, *Electric walls*, *Armored portals*.
5. **Controls row:** keycap-style hints — `← ↑ ↓ →` move, `W A S D` shoot, `Space` run, `Esc` give up.
6. **"Press Return to start".**
7. **Credits at the bottom**, two lines:
   `© 1982 SuperSet Software Corp. All rights reserved.`
   `Ported by David Ellsworth · macOS port by mrBallistic`

### What the level card says

All values come from the existing tables in `Snipes.cpp` (do not duplicate them; read the same arrays):

- **Number (1–9):** snipes = `maxSnipesTable` (10–150; this is the maximum alive at once, so say "snipes", not a total), portals = `numSnipePortalsTable` (3–10), lives = `numLivesTable` (5 down to 2).
- **Letter (A–Z):** small snipes = `enableSmallSnipesTable`; bouncing bullets = `bouncingBulletTable`; electric walls = letter ≥ M; armored portals (portals resist your shots) = letter ≥ W. These mirror the assignments at the start of each match (`enableElectricWalls = skillLevelLetter >= 'M'-'A'`, etc.).
- Snipe aim (`snipeShootingAccuracyTable`) varies unevenly across letters. Showing it (for example as a small meter in the card) is the builder's call; the mockup omits it.

### Behaviour

- ← / → change the letter, ↑ / ↓ change the number; neither wraps (my call; wrapping is also acceptable).
- Typing a letter or a digit jumps straight to it, so players who know their level can type `B3` as before.
- **Return** starts the game.
- **Esc** on the start screen quits the app (my call).
- The screen starts on the **last level played**, remembered across launches (my call). First launch defaults to A1, matching the current default (`skillLevelLetter = 0`, `skillLevelNumber = 1`).
- After a game ends and the player answers **Y** to "Play another game?", they return to this screen, with the level they just played selected, instead of the old `Enter new skill level` prompt. **N** quits as now.
- Replay playback (`snipes <file>`) skips the start screen, as it skips the old title today.
- The window and resizing behave as they do now; the screen lays itself out relative to the window size.

## The in-game bar (decided)

Mockup `hud-final-v4.html`.

- **One slim row**, about half the height of today's three-row bar, same dark-navy background as the start screen with a thin divider line under it.
- **The playfield keeps its exact size** (22 rows × 40 columns at the current tile size). The window becomes shorter by the height saved; it does not show more maze.
- Left to right, label then value:
  - **Snipes** — a single number: big plus small snipes currently alive (`numLargeSnipes + numSmallSnipes`). No "of N" total: portals keep spawning snipes, so the count rises and falls.
  - **Portals** — portals remaining (`numSnipePortals`), plus a row of small pips, one per starting portal (`numSnipePortalsAtStart`), lit for those remaining.
  - **Time** — game time as minutes and seconds (`1:24`), derived from `frame` (see below).
  - **Score** — six digits, zero-padded as today.
  - Right-aligned, smaller and dimmer: **lives remaining** as small yellow ☺ faces in the game font, and the **level code** (e.g. `E5`). Lives and level weren't asked for but are kept, de-emphasised, because losing your last life without warning would be worse (my call).
- The bar shows during replay playback too.
- Killed counts (the old cyan numbers) are dropped.
- **Fixed-width slots, generously spaced.** Each value gets a slot sized for its largest possible value, so nothing shifts as numbers change: snipes 3 digits (the count is capped by `maxSnipes`, at most 150, and stored in a `BYTE`, so at most 255), portals 2 digits plus up to 10 pips, time `59:59`, score 6 digits. Use tabular (fixed-width) figures. Groups are spaced well apart (the mockup's ~24px gap at 720px width, scaling with the window), and the pip row always reserves room for 10 pips so later groups never shift. The bar must still fit with every slot at its maximum; the mockup shows this.

## Constraints

- **Threading.** On macOS the SDL window, rendering and event loop run on the main thread (`ConsoleThreadFunc`, called from `main()` in `sdl/console.cpp`) and the game logic runs on a worker thread (`GameMain`). The game thread currently drives the title by writing text into the shared `Screen` grid and blocking in `ReadTextFromConsole`. The new start screen must keep that split: the main thread draws it and handles its input; the game thread blocks until a level is chosen (or quit), then continues. Share state under `ScreenMutex` or atomics, as the existing code does. The non-Apple SDL path (where `OpenConsole` spawns a render thread) must keep working.
- **The game logic keeps writing the old HUD.** `InitializeHUD` and `UpdateHUD` write the old bar into rows 0–2 of `Screen`. Leave that code path alone (it's shared with the console build and its timing is part of the frame loop); the SDL renderer should stop drawing rows 0–2 and draw the new bar instead, from values the game thread publishes. Make sure nothing else the player needs is ever written into rows 0–2 during play.
- **Retina.** The window is currently created without high-DPI support, so it renders at point resolution and macOS upscales it. Smooth UI text must render at the display's full pixel density. Turning on high-DPI may also change how the playfield glyphs render (likely sharper); that's acceptable as long as layout and proportions don't change.
- **Fonts.** The SNIPES art, lives faces and any game glyphs use `SnipesConsole.ttf` via the existing loader (`OpenGameFont`) and its fallbacks. UI text needs a clean sans-serif. On macOS the system font is fine; if a font is bundled it must be freely redistributable (e.g. SIL OFL) and added to the app bundle by `scripts/make-macos-app.sh`.
- **User-facing copy uses smart punctuation:** curly quotes and apostrophes (’ “ ”), en dashes in ranges (A–Z, 1–9), the © sign. No straight `'` or `"` in rendered text.
- **Build.** The Makefile build (`make`), `scripts/make-macos-app.sh` (signing, bundling, notarization) and the existing behaviour of `config.h` options (`TILE_HEIGHT`, `TILE_WIDTH`, `FONT_SIZE`, `WINDOW_SCREEN_FRACTION`) must keep working. The window's initial size is still chosen from `WINDOW_SCREEN_FRACTION`; with a shorter bar it's just a little shorter.

## Time display (decided)

Show minutes and seconds. `frame` advances once per game tick, and `GetTickCountWord()` in `sdl/timer.cpp` (`SDL_GetTicks() * 13125 / (11*65535)`) fixes that at the PC timer rate, about 18.2 per second, so seconds = `frame * 65536 / 1193182`. Display only; `frame` itself is untouched. Accepted consequences:

- It is game time, not wall-clock time: it stops while the window is unfocused and runs fast during replay fast-forward.
- `frame` is a `WORD`, so it wraps after about an hour and the clock jumps from about 59:59 back to 0:00. The raw counter wraps at the same point today.

## Other platforms (my call)

All SDL builds (macOS, Linux, Windows SDL) get the new screens; add any new `sdl/` files to `Snipes.vcxproj`/`.filters` as well as the Makefile. UI text uses a system sans-serif found by a fallback chain like `OpenGameFont`'s: Helvetica Neue or SF on macOS, DejaVu Sans on Linux, Segoe UI on Windows. No font is bundled for the UI.

## Left to the builder

- Exact sizes, spacing, glow, and type scale, matching the mockups' look and scaling with the window.
- Whether to show snipe aim on the level card, and how.
- How the start screen and bar are structured in code (new files under `sdl/` are fine), how the last-played level is stored (a small file under the app's preferences directory, `SDL_GetPrefPath`, is the obvious choice), and how the game thread hands off to the start screen.
- Any transition (e.g. a short fade) from start screen into the game.
