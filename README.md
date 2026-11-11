# Snipes

This is a modern port of the classic 1982 text-mode game Snipes. The code has been reverse-engineered from the original DOS executable, and has 100% identical game logic. 

For more information, see the [vogons.org forum thread](https://www.vogons.org/viewtopic.php?f=7&t=49073).

This is a fork of [Davidebyzero/Snipes](https://github.com/Davidebyzero/Snipes) that adds a native macOS app. It also:

- a graphical start screen that explains the controls and what each level means
- a slim status bar showing snipes and portals left, time and score
- sharp, full-resolution graphics on Retina displays
- a window that opens at a comfortable size instead of the full screen height

The game logic is unchanged, so replays recorded with the original port still play back identically.

<p align="center">
  <img src="docs/screenshots/start-screen.png" alt="The start screen: the SNIPES title art above strips for choosing the enemy skill (A to Z) and enemy numbers (1 to 9), with level M4 selected and its details shown" width="49%">
  <img src="docs/screenshots/gameplay.png" alt="Gameplay at level M9: the status bar shows 16 snipes and 10 portals left above the maze, with a snipe and a portal near the player" width="49%">
</p>

### Download for macOS

**[Download the latest release](https://github.com/mrballistic/Snipes/releases/latest)**: open the `.dmg` and drag Snipes into your Applications folder. (A `.zip` of the app is there too.)

- It's signed with a Developer ID and notarized by Apple, so it opens without a Gatekeeper warning.
- It's self-contained: SDL and the original font are bundled, so you don't need Homebrew.
- It runs on Apple Silicon and Intel Macs with macOS 15 or later.
- Replays are saved to `~/Library/Application Support/Snipes`.

To build it yourself, on macOS, Linux or Windows, see [BUILDING.md](BUILDING.md).

### Playing

On the start screen, pick a level and press Return. A level is a letter and a number, such as `M4`:

- **The letter (A–Z) sets how dangerous the snipes are:** how well they aim, whether small snipes appear, whether bullets bounce off walls, whether walls are electrified (M and up), and whether portals shrug off your shots (W and up).
- **The number (1–9) sets how many you face:** from 10 snipes and 3 portals with 5 lives, up to 150 snipes and 10 portals with 2 lives.

Use ← → and ↑ ↓ to move through the strips, or type the level (for example `b3`). The card below them spells out what the level means. The start screen remembers the last level you played.

In the game, the arrow keys move and W A S D shoot (X also shoots down). Hold Space to move faster, though you can’t shoot while it’s held. Esc gives up the current game. F1 turns sound on and off, and F2 turns the shooting sound on and off.

### Replay recording

This version automatically records replay files of played games. Replay files have a `.SnipesGame` file extension. They're saved to the current directory, or to `~/Library/Application Support/Snipes` when you run the macOS app.

To play back a replay, pass the file to the game from Terminal. For the macOS app:

```
/Applications/Snipes.app/Contents/MacOS/Snipes "2016-07-08 09.10.11.SnipesGame"
```

For a build from source, use `./snipes "2016-07-08 09.10.11.SnipesGame"`.

### License

As this is a reverse-engineered port, copyright is retained by the original authors of Snipes.

This reverse-engineered source code is released with the original authors' permission.

The start screen and status bar use [IBM Plex Mono](https://github.com/IBM/plex), licensed under the SIL Open Font License.
