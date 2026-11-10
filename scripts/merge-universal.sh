#!/usr/bin/env bash
# Merges an Apple Silicon and an Intel build of Snipes.app (each from
# `scripts/make-macos-app.sh --no-sign`) into one unsigned universal app.
#
#   scripts/merge-universal.sh <arm64 Snipes.app> <x86_64 Snipes.app> <output Snipes.app>
#
# The executable and every bundled library present in both builds are combined
# with lipo. A library only one build bundles is copied as-is: only that
# architecture's code loads it. The app's minimum macOS is the newer of the two.
set -euo pipefail

ARM=${1:?usage: $0 <arm64 app> <x86_64 app> <output app>}
X86=${2:?usage: $0 <arm64 app> <x86_64 app> <output app>}
OUT=${3:?usage: $0 <arm64 app> <x86_64 app> <output app>}

check_arch() {
	local archs
	archs=$(lipo -archs "$1/Contents/MacOS/Snipes")
	[[ "$archs" == "$2" ]] || { echo "error: $1 is $archs, expected $2" >&2; exit 1; }
}
check_arch "$ARM" arm64
check_arch "$X86" x86_64

rm -rf "$OUT"
ditto "$ARM" "$OUT"

for rel in Contents/MacOS/Snipes $(cd "$X86" && ls Contents/Frameworks/*.dylib); do
	if [[ -f "$OUT/$rel" ]]; then
		lipo -create "$ARM/$rel" "$X86/$rel" -output "$OUT/$rel.universal"
		mv "$OUT/$rel.universal" "$OUT/$rel"
	else
		echo "note: $(basename "$rel") is only bundled by the Intel build"
		cp "$X86/$rel" "$OUT/$rel"
	fi
done
for f in "$OUT"/Contents/Frameworks/*.dylib; do
	[[ -f "$X86/Contents/Frameworks/$(basename "$f")" ]] || echo "note: $(basename "$f") is only bundled by the Apple Silicon build"
done

min_arm=$(plutil -extract LSMinimumSystemVersion raw "$ARM/Contents/Info.plist")
min_x86=$(plutil -extract LSMinimumSystemVersion raw "$X86/Contents/Info.plist")
min=$(printf '%s\n%s\n' "$min_arm" "$min_x86" | sort -V | tail -1)
plutil -replace LSMinimumSystemVersion -string "$min" "$OUT/Contents/Info.plist"

echo "==> Universal app: $OUT ($(lipo -archs "$OUT/Contents/MacOS/Snipes"); macOS $min or later)"
