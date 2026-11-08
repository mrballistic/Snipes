#!/usr/bin/env bash
# Captures the README screenshots from the built app (run scripts/make-macos-app.sh first):
#   docs/screenshots/start-screen.png  the start screen with level M4 selected
#   docs/screenshots/gameplay.png      level M9 (the most portals), well under way
#
#   scripts/capture-screenshots.sh
#
# The script types into the game window, so leave the keyboard alone while it
# runs (about a minute). It only sends keys while the game is the frontmost app,
# and it restores your remembered start-screen level afterwards.
set -euo pipefail

cd "$(dirname "$0")/.."
APP_BIN=dist/Snipes.app/Contents/MacOS/Snipes
OUT=docs/screenshots
SAVED_LEVEL="$HOME/Library/Application Support/Snipes/level.txt"
GAMEPLAY_SECONDS=${GAMEPLAY_SECONDS:-45}

[[ -x "$APP_BIN" ]] || { echo "error: $APP_BIN not found; run scripts/make-macos-app.sh first" >&2; exit 1; }
mkdir -p "$OUT"

backup=""
if [[ -f "$SAVED_LEVEL" ]]; then backup=$(cat "$SAVED_LEVEL"); fi
restore() {
	if [[ -n "$backup" ]]; then printf '%s\n' "$backup" > "$SAVED_LEVEL"; fi
	[[ -n "${pid:-}" ]] && kill "$pid" 2>/dev/null || true
}
trap restore EXIT

frontmost() { osascript -e 'tell application "System Events" to get name of first process whose frontmost is true'; }
focus() {
	for _ in 1 2 3 4 5 6 7 8 9 10; do
		osascript -e 'tell application "System Events" to set frontmost of process "Snipes" to true' 2>/dev/null || true
		sleep 0.5
		[[ "$(frontmost)" == "Snipes" ]] && return 0
	done
	echo "error: couldn't bring Snipes to the front; is another app holding focus?" >&2
	exit 1
}
# Sends macOS virtual key codes, refusing if focus has moved to another app.
keys() {
	for k in "$@"; do
		[[ "$(frontmost)" == "Snipes" ]] || { echo "error: Snipes lost focus; not typing into another app" >&2; exit 1; }
		osascript -e "tell application \"System Events\" to key code $k"
		sleep 0.2
	done
}
capture() {
	local id
	id=$(swift scripts/window-id.swift Snipes)
	screencapture -o -x -l "$id" "$1"
	sips -Z 1600 "$1" >/dev/null # Retina captures are large; 1600px is plenty for a README
	echo "captured $1"
}

# Keep the game running if focus wanders while waiting for snipes to appear
SNIPES_NO_AUTOPAUSE=1 "$APP_BIN" >/dev/null 2>&1 &
pid=$!
sleep 3

focus
keys 46 21 # "m", "4"
sleep 0.5
capture "$OUT/start-screen.png"

keys 25 36 # "9" for the busiest maze, then Return
sleep "$GAMEPLAY_SECONDS"
capture "$OUT/gameplay.png"
