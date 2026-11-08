# Start Screen and In-Game Bar Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the SDL build's text title screen and three-row status bar with a graphical start screen and a slim graphical bar, without changing game logic or replays.

**Architecture:** The game thread keeps running `GameMain` unchanged except at two seams: it calls a new blocking `ChooseSkillLevel()` instead of printing the title and reading a skill string, and it publishes HUD values with `PublishHud()` from `InitializeHUD`/`UpdateHUD`. The SDL render loop (main thread on macOS) draws either the start screen or the playfield; during play it skips grid rows 0–2 and draws the bar from the published values. Pure logic (level descriptions, time formatting, start-screen key handling, saved-level parsing) lives in small files with a unit-test binary.

**Tech Stack:** C++11, SDL2 (via sdl2-compat on macOS) and SDL2_ttf, GNU make; MSVC project for Windows.

**Spec:** `docs/superpowers/specs/2026-11-07-start-screen-and-hud-design.md`

## Global Constraints

- No change to game logic, timing, `frame`, or the replay file format; existing `.SnipesGame` files must play back identically.
- Playfield stays 22 rows × 40 columns, same font, colors and tile size.
- Windows console build (`_CONSOLE`) keeps the old text title and status bar; every new call site in `Snipes.cpp` is guarded by `#ifndef _CONSOLE`.
- Rendered copy uses smart punctuation: `’ “ ”`, en dash in ranges (`A–Z`, `1–9`), `©`.
- Credits, exactly: `© 1982 SuperSet Software Corp. All rights reserved.` / `Ported by David Ellsworth · macOS port by mrBallistic`.
- Text must be sharp on Retina: the window is created with `SDL_WINDOW_ALLOW_HIGHDPI` and everything renders at drawable (pixel) resolution.
- UI font: system sans-serif via fallback chain (macOS Helvetica Neue / SF, Linux DejaVu Sans, Windows Segoe UI); game glyphs keep `SnipesConsole.ttf` via `OpenGameFont`.
- `make`, `scripts/make-macos-app.sh`, and `config.h` options (`TILE_HEIGHT`, `TILE_WIDTH`, `FONT_SIZE`, `WINDOW_SCREEN_FRACTION`) keep working.
- Colors from the mockups: background `#05070d`, bar background `#0b1020`, divider `#1d2740`, accent `#7fe7ff`, label gray `#8f99b3`, off-chip `#18213a`, lit-chip `#123a2c` on `#6ff0b0`, lives `#ffd36b`.

## Review Focus

1. **Esc / window close on the start screen.** Esc quits the app; closing the window quits; neither leaves the game thread blocked. Pinned by Task 3's `StartScreenKey` Esc test and Task 4's manual check.
2. **Key events leaking across the screen boundary.** The Return that starts a game, or a letter typed on the start screen, must not register as a held key or a shot in the first frame of play; Esc pressed during play must still forfeit. Task 4 routes start-screen keys away from `HandleKey`, and its manual check covers it.
3. **Corrupt or missing saved level.** A missing, empty or garbage `level.txt` yields A1 without crashing. Pinned by Task 3's `ParseSavedLevel` tests.
4. **Max values in the bar.** 150+ snipes, 10 portals, `59:59`, 6-digit score at the smallest window size still fit without overlap. Pinned by Task 2's `FormatGameTime` wrap test and Task 5's manual max-value check.
5. **Resizing.** Dragging the window (on either screen) keeps proportions, keeps the bar exactly as tall as designed, and re-renders sharp text. Task 5's manual check.

---

### Task 1: Move skill tables into `skill.h/.cpp` with a description function

**Files:**
- Create: `skill.h`, `skill.cpp`, `tests/test_main.cpp`
- Modify: `Snipes.cpp:113-119` (tables move out; uses unchanged), `GNUmakefile`, `Snipes.vcxproj`, `Snipes.vcxproj.filters`

**Interfaces:**
- Produces: the seven tables as `extern const` arrays with the same names and types (`snipeShootingAccuracyTable`, `enableSmallSnipesTable`, `bouncingBulletTable`, `smallSnipeExplosionChanceTable`, `maxSnipesTable`, `numSnipePortalsTable`, `numLivesTable`), and
  ```cpp
  struct SkillLevelInfo { Uint snipes, portals, lives, aim; bool smallSnipes, bouncingBullets, electricWalls, armoredPortals; };
  SkillLevelInfo DescribeSkillLevel(Uint letter /*0–25*/, Uint number /*1–9*/);
  ```
  `aim` = `snipeShootingAccuracyTable[letter]`; `electricWalls` = `letter >= 'M'-'A'`; `armoredPortals` = `letter >= 'W'-'A'`.

- [ ] **Step 1: Write failing tests** in `tests/test_main.cpp` (tiny `CHECK(cond)` macro that counts failures; `main` returns nonzero on any failure):
  - A1: snipes 10, portals 3, lives 5, no hazards.
  - E5: snipes 60, portals 5, lives 5, smallSnipes true, bouncing false, electric false.
  - M4: electricWalls true, armoredPortals false. L: electric false.
  - W9: armoredPortals true, snipes 150, portals 10, lives 2. V: armored false.
  - G1: bouncingBullets true.
- [ ] **Step 2: Add `make test`** (builds `tests/test_main.cpp skill.cpp` without SDL into `snipes-tests`, runs it). Run `make test`; expect a link failure for `DescribeSkillLevel`.
- [ ] **Step 3: Implement** — move the tables verbatim into `skill.cpp`, include `skill.h` from `Snipes.cpp`; add `skill.cpp` to `SOURCES` and to the vcxproj (all configurations).
- [ ] **Step 4: Verify** `make test` passes and `make` builds `snipes` with no new warnings.
- [ ] **Step 5: Commit** `[Snipes] Move skill tables to skill.cpp and describe levels`.

### Task 2: Time formatting

**Files:** Create `sdl/hudformat.h`, `sdl/hudformat.cpp`; Modify `tests/test_main.cpp`, `GNUmakefile`.

**Interfaces:**
- Produces: `void FormatGameTime(WORD frame, char out[8]);` → `"m:ss"` / `"mm:ss"`, seconds = `frame * 65536 / 1193182` (integer).

- [ ] **Step 1: Tests:** frame 0 → `"0:00"`; 18 → `"0:00"`; 19 → `"0:01"`; 1093 → `"1:00"`; 65535 → `"59:59"`.
- [ ] **Step 2:** `make test` fails to link. **Step 3:** implement. **Step 4:** `make test` passes. **Step 5:** commit `[SDL] Add game-time formatting for the new bar`.

### Task 3: Start-screen logic and saved level

**Files:** Create `sdl/startscreen_logic.h`, `sdl/startscreen_logic.cpp`; Modify `tests/test_main.cpp`, `GNUmakefile`.

**Interfaces:**
- Produces:
  ```cpp
  struct StartSelection { Uint letter; Uint number; };            // 0–25, 1–9
  enum StartKey { StartKey_Left, StartKey_Right, StartKey_Up, StartKey_Down, StartKey_Return, StartKey_Escape, StartKey_Char };
  enum StartAction { StartAction_None, StartAction_Start, StartAction_Quit };
  StartAction StartScreenKey(StartSelection &sel, StartKey key, char chr = 0);
  bool ParseSavedLevel(const char *text, StartSelection &sel);    // "M4\n" → true
  void FormatSavedLevel(const StartSelection &sel, char out[4]);  // → "M4"
  ```
  Left/Right change letter, Up/Down change number, clamped (no wrap). `StartKey_Char` with `a–z`/`A–Z` sets letter, `1–9` sets number, anything else ignored. Return → Start, Escape → Quit.

- [ ] **Step 1: Tests:** Right from A → B; Left from A stays A; Right from Z stays Z; Up from 1 → 2; Up from 9 stays 9; Down from 1 stays 1; Char 'm' → letter 12; Char '7' → number 7; Char '0' and '#' ignored; Return → Start; Escape → Quit. `ParseSavedLevel`: `"M4"` and `"m4\n"` → M4 true; `""`, `"Z0"`, `"44"`, `"M"`, `NULL` → false and sel unchanged. `FormatSavedLevel` round-trips.
- [ ] **Steps 2–4:** fail, implement, pass. **Step 5:** commit `[SDL] Add start-screen selection logic and saved level parsing`.

### Task 4: Start screen rendering and game-thread handoff

**Files:** Create `sdl/uifont.h/.cpp`, `sdl/startscreen.h/.cpp`; Modify `sdl/console.cpp` (render loop, event routing, HiDPI), `console.h`, `Snipes.cpp` (title block ~2195-2232, play-again ~2494-2516), `GNUmakefile`, vcxproj/filters (SDL configs only, mirroring `sdl\console.cpp` exclusions).

**Interfaces:**
- Consumes: Task 1 `DescribeSkillLevel`; Task 3 `StartScreenKey`, `ParseSavedLevel`, `FormatSavedLevel`.
- Produces:
  - `console.h`: `bool ChooseSkillLevel(Uint &letter, Uint &number);` — blocks the game thread; returns false on quit/close. Loads the saved level as the initial selection when called first in a run, otherwise uses the passed-in level; saves `level.txt` under `SDL_GetPrefPath("", "Snipes")` on Start.
  - `sdl/uifont.h`: `TTF_Font *GetUIFont(int pixelSize, bool bold);` cached by (size, bold); `void CloseUIFonts();`.
  - `sdl/startscreen.h`: `void DrawStartScreen(SDL_Renderer *ren, int w, int h, const StartSelection &sel, TTF_Font *gameFont);` draws the layout from `start-screen-a-v2.html`, scaled to `w×h` pixels.
- Behaviour: while the start screen is active, the render loop draws only it, and keydown events go to `StartScreenKey` instead of `HandleKey` (keyups still go to `HandleKey` so no key stays latched). `SDL_QUIT` keeps its current flags so the game exits. In `Snipes.cpp`, `#ifndef _CONSOLE` replaces the title text + `ReadSkillLevel()` with `if (!ChooseSkillLevel(skillLevelLetter, skillLevelNumber)) goto do_not_play_again;` and replaces the "Enter new skill level" prompt with `ClearConsole();` + the same call (quit → `break`).
- HiDPI: create the window with `SDL_WINDOW_ALLOW_HIGHDPI`; compute `PixelScale = drawableHeight / windowHeight`; `TileWidth/TileHeight/FontSize` become pixel values (× `PixelScale`) and window sizes divide by it.

- [ ] **Step 1:** Implement fonts, drawing, handoff and HiDPI as above.
- [ ] **Step 2: Verify** `make test` still passes; `make` builds; launch `./snipes`: the start screen matches the mockup, ←→↑↓ and typed `b3` move the selection, hazard tags light per level (check A1, E5, M4, W9), Return starts at the chosen level, Esc on the start screen quits, closing the window quits, after a game Y returns to the start screen with that level selected, relaunch remembers it. Esc during play still forfeits. Play back the replay in `~/Library/Application Support/Snipes` with `./snipes <file>`: no start screen. Screenshot and inspect each state.
- [ ] **Step 3: Commit** `[SDL] Replace the title screen with a graphical start screen`.

### Task 5: Slim in-game bar

**Files:** Create `sdl/hud.h`, `sdl/hud.cpp`; Modify `sdl/console.cpp` (layout + sizing), `Snipes.cpp` (`InitializeHUD`, `UpdateHUD`), `console.h`, `GNUmakefile`, vcxproj/filters.

**Interfaces:**
- Consumes: Task 2 `FormatGameTime`; Task 4 `GetUIFont`.
- Produces:
  - `console.h`: `struct HudValues { Uint snipes, portals, portalsAtStart, score, lives, letter, number; WORD frame; };` and `void PublishHud(const HudValues &v);` (copies under `ScreenMutex`). Called at the end of `InitializeHUD` and `UpdateHUD` under `#ifndef _CONSOLE`, with `snipes = numLargeSnipes + numSmallSnipes`, `portals = numSnipePortals`, `score = score * 10` (the old bar's trailing 0), `lives = numLives - numPlayerDeaths`.
  - `sdl/hud.h`: `void DrawHud(SDL_Renderer *ren, int w, int barHeight, const HudValues &v, TTF_Font *gameFont);` — layout from `hud-final-v4.html`: fixed-width slots (snipes 3 digits, portals 2 + 10-pip slot, time `59:59`, score 6 digits), lives faces + level right-aligned.
- Layout: bar height `HudHeight = round(1.5 × TileHeight)`; playfield grid row `r ≥ 3` draws at `y = HudHeight + (r − 3) × TileHeight`; rows 0–2 are not drawn; the blinking cursor uses the same offset. Window height = `HudHeight + VIEWPORT_HEIGHT × TileHeight` everywhere `WINDOW_HEIGHT × TileHeight` was used (initial sizing and the resize handler).

- [ ] **Step 1:** Implement publishing, drawing and layout.
- [ ] **Step 2: Verify** `make test` passes; `make` builds with no new warnings; in-game the bar matches the mockup and updates live (snipes rise/fall, portal pips drop, time ticks m:ss, score), the playfield is unchanged in size, the end-of-game text still appears at the bottom. Temporarily force max values (`snipes 255, portals 10, frame 65535, score 999999`) and shrink the window to its smallest size: nothing overlaps; revert. Resize the window: bar keeps proportion, text stays sharp. Screenshot each.
- [ ] **Step 3: Commit** `[SDL] Replace the status rows with a slim graphical bar`.

### Task 6: Package, screenshots and document

**Files:** Create `docs/screenshots/start-screen.png`, `docs/screenshots/gameplay.png`, `scripts/capture-screenshots.sh`; Modify `README.md`; run `scripts/make-macos-app.sh`.

- [ ] **Step 1: Screenshots.** `scripts/capture-screenshots.sh` launches the built app, finds its window ID (`osascript`/`GetWindowList` via Swift one-liner), and captures just the window with `screencapture -o -l <id>` (no shadow). Capture the start screen with an interesting level selected (e.g. M4, so a hazard tag is lit) and gameplay a few seconds into a game with snipes visible and the bar populated. Inspect both images.
- [ ] **Step 2:** README: add both screenshots near the top (under the download section), replace the controls/skill wording with the new start screen (controls, what A–Z and 1–9 mean), mention the bar.
- [ ] **Step 3: Verify** `NOTARY_PROFILE=snipes-notary scripts/make-macos-app.sh` ends with `accepted` / `source=Notarized Developer ID`; launch the app and repeat the start-screen and bar checks once.
- [ ] **Step 4: Commit** `[README] Describe the new start screen and bar, add screenshots`, push to `fork` (`macos-main-thread` and `master`) using the `mrballistic` gh account.
