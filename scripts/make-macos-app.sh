#!/usr/bin/env bash
# Builds a self-contained, signed Snipes.app (and a zip of it) in dist/.
#
#   scripts/make-macos-app.sh              build for this Mac's architecture, then sign and package
#   scripts/make-macos-app.sh --no-sign    build and assemble only (used by CI; see .github/workflows)
#   scripts/make-macos-app.sh --app PATH   sign and package an existing app, e.g. the universal
#                                          build from CI (see BUILDING.md)
#
# Environment:
#   SIGN_IDENTITY   codesign identity (default: first "Developer ID Application" in the keychain)
#   NOTARY_PROFILE  notarytool keychain profile; if set, the app is notarized and stapled.
#                   Create one with:
#                     xcrun notarytool store-credentials <name> --apple-id <id> --team-id <team>
#   BUNDLE_ID       CFBundleIdentifier (default: com.mrballistic.snipes)
#   MACOSX_DEPLOYMENT_TARGET  oldest macOS to compile for; the app's minimum is the newest
#                   that any bundled binary needs (Homebrew's libraries target the OS they were built on)
#
# Also writes a drag-to-Applications disk image if create-dmg is installed
# (brew install create-dmg).
#
# Requires Homebrew's sdl2-compat (or sdl2), sdl3 and sdl2_ttf. Put SnipesConsole.ttf in the
# repo root to bundle the authentic font; otherwise the game falls back to Menlo.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT=$PWD
DIST=$ROOT/dist
APP=$DIST/Snipes.app
BUNDLE_ID=${BUNDLE_ID:-com.mrballistic.snipes}

SIGN=1
SOURCE_APP=
while [[ $# -gt 0 ]]; do
	case "$1" in
		--no-sign) SIGN=0 ;;
		--app) SOURCE_APP=${2:?--app needs a path}; shift ;;
		*) echo "usage: $0 [--no-sign | --app PATH]" >&2; exit 2 ;;
	esac
	shift
done

# Newest LC_BUILD_VERSION minos across every architecture slice of the given Mach-O files
max_minos() {
	local f
	for f in "$@"; do otool -arch all -l "$f" | awk '/LC_BUILD_VERSION/{b=1} b&&/minos/{print $2; b=0}'; done | sort -V | tail -1
}

if [[ $SIGN == 1 && -z "${SIGN_IDENTITY:-}" ]]; then
	SIGN_IDENTITY=$(security find-identity -v -p codesigning | sed -n 's/.*"\(Developer ID Application:[^"]*\)".*/\1/p' | head -1)
	[[ -n "$SIGN_IDENTITY" ]] || { echo "No Developer ID Application identity found; set SIGN_IDENTITY" >&2; exit 1; }
fi

if [[ -n "$SOURCE_APP" ]]; then
	echo "==> Using $SOURCE_APP"
	[[ -d "$SOURCE_APP" ]] || { echo "error: $SOURCE_APP not found" >&2; exit 1; }
	rm -rf "$APP"
	mkdir -p "$DIST"
	ditto "$SOURCE_APP" "$APP"
	VERSION=$(plutil -extract CFBundleShortVersionString raw "$APP/Contents/Info.plist")
	FW=$APP/Contents/Frameworks
else
	VERSION=${VERSION:-1.0.$(git rev-list --count HEAD)}
	echo "==> Building"
	rm -f snipes
	make

	echo "==> Assembling $APP"
	rm -rf "$APP"
	mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources" "$APP/Contents/Frameworks"
	cp snipes "$APP/Contents/MacOS/Snipes"

	# IBM Plex Mono (SIL OFL) for the start screen and the in-game bar, with its license
	mkdir -p "$APP/Contents/Resources/fonts"
	cp fonts/IBMPlexMono-Regular.ttf fonts/IBMPlexMono-Bold.ttf fonts/IBMPlexMono-LICENSE.txt "$APP/Contents/Resources/fonts/"

	if [[ -f SnipesConsole.ttf ]]; then
		cp SnipesConsole.ttf "$APP/Contents/Resources/"
	else
		echo "warning: SnipesConsole.ttf not found; the app will use a fallback font" >&2
	fi

	echo "==> Bundling dylibs"
	FW=$APP/Contents/Frameworks

	# Copies every non-system dylib $1 links against into Frameworks (recursively)
	# and rewrites the references to point at the bundled copies. $2 is the
	# directory $1 originally came from, used to resolve @rpath/@loader_path refs.
	bundle_deps() {
		local bin=$1 srcdir=$2 ref name src
		while read -r ref; do
			name=$(basename "$ref")
			[[ "$name" == "$(basename "$bin")" ]] && continue # the library's own id
			case "$ref" in
				@rpath/*|@loader_path/*) src=$srcdir/$name ;;
				*) src=$ref ;;
			esac
			if [[ ! -f "$FW/$name" ]]; then
				[[ -f "$src" ]] || { echo "error: can't find $ref (needed by $bin)" >&2; exit 1; }
				cp -L "$src" "$FW/$name"
				chmod u+w "$FW/$name"
				install_name_tool -id "@rpath/$name" "$FW/$name"
				bundle_deps "$FW/$name" "$(dirname "$(realpath "$src")")"
			fi
			[[ "$ref" == "@rpath/$name" ]] || install_name_tool -change "$ref" "@rpath/$name" "$bin"
		done < <(otool -L "$bin" | tail -n +2 | awk '{print $1}' | grep -E '^(/opt/|/usr/local/|@rpath/|@loader_path/)' || true)
	}

	install_name_tool -add_rpath "@executable_path/../Frameworks" "$APP/Contents/MacOS/Snipes"
	bundle_deps "$APP/Contents/MacOS/Snipes" "$ROOT"

	# sdl2-compat dlopen()s SDL3 at runtime rather than linking it, looking first
	# for @loader_path/libSDL3.dylib, so ship it beside libSDL2 under that name.
	if strings "$FW/libSDL2-2.0.0.dylib" | grep -q '@loader_path/libSDL3.dylib'; then
		SDL3_LIB=$(brew --prefix sdl3)/lib/libSDL3.0.dylib
		cp -L "$SDL3_LIB" "$FW/libSDL3.dylib"
		chmod u+w "$FW/libSDL3.dylib"
		install_name_tool -id "@rpath/libSDL3.dylib" "$FW/libSDL3.dylib"
		bundle_deps "$FW/libSDL3.dylib" "$(dirname "$(realpath "$SDL3_LIB")")"
	fi

	# Bundled dylibs find each other via @rpath, so each needs an rpath to its own directory.
	for f in "$FW"/*.dylib; do
		otool -l "$f" | grep -q 'path @loader_path ' || install_name_tool -add_rpath "@loader_path" "$f"
	done

	# Fail if anything still points outside the bundle.
	for f in "$APP/Contents/MacOS/Snipes" "$FW"/*.dylib; do
		if otool -L "$f" | tail -n +2 | grep -E '^\s*/(opt|usr/local)/'; then
			echo "error: $f still references a non-bundled library" >&2
			exit 1
		fi
	done

	# The app can only run where every bundled binary can
	MIN_MACOS=$(max_minos "$APP/Contents/MacOS/Snipes" "$FW"/*.dylib)
	echo "==> Minimum macOS: $MIN_MACOS"

	# App icon: art/Icon.icon (Icon Composer), or macos/AppIcon.xcassets
	# (Xcode asset catalog containing an AppIcon set), or a ready-made macos/AppIcon.icns.
	ICON_KEYS=
	ICON_SRC=
	for c in art/Icon.icon macos/AppIcon.xcassets; do [[ -e "$c" ]] && { ICON_SRC=$c; break; }; done
	if [[ -n "$ICON_SRC" ]]; then
		# actool names the icon after the .icon file, or after the asset catalog's icon set
		ICON_NAME=$([[ "$ICON_SRC" == *.icon ]] && basename "$ICON_SRC" .icon || echo AppIcon)
		xcrun actool "$ICON_SRC" --compile "$APP/Contents/Resources" --app-icon "$ICON_NAME" \
			--platform macosx --minimum-deployment-target "$MIN_MACOS" \
			--output-partial-info-plist "$DIST/icon-partial.plist" >/dev/null
		rm -f "$DIST/icon-partial.plist"
		ICON_KEYS="<key>CFBundleIconFile</key><string>$ICON_NAME</string><key>CFBundleIconName</key><string>$ICON_NAME</string>"
	elif [[ -f macos/AppIcon.icns ]]; then
		cp macos/AppIcon.icns "$APP/Contents/Resources/"
		ICON_KEYS="<key>CFBundleIconFile</key><string>AppIcon</string>"
	else
		echo "note: no icon in macos/; the app will use the generic icon"
	fi

	cat > "$APP/Contents/Info.plist" <<-EOF
	<?xml version="1.0" encoding="UTF-8"?>
	<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
	<plist version="1.0">
	<dict>
		<key>CFBundleName</key><string>Snipes</string>
		<key>CFBundleDisplayName</key><string>Snipes</string>
		<key>CFBundleExecutable</key><string>Snipes</string>
		<key>CFBundleIdentifier</key><string>$BUNDLE_ID</string>
		<key>CFBundlePackageType</key><string>APPL</string>
		<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>
		<key>CFBundleShortVersionString</key><string>$VERSION</string>
		<key>CFBundleVersion</key><string>$VERSION</string>
		<key>LSApplicationCategoryType</key><string>public.app-category.arcade-games</string>
		<key>LSMinimumSystemVersion</key><string>$MIN_MACOS</string>
		<key>NSHighResolutionCapable</key><true/>
		<key>NSPrincipalClass</key><string>NSApplication</string>
		$ICON_KEYS
	</dict>
	</plist>
	EOF


fi

# "arm64", "x86_64", or "universal" for both
ARCHS=$(lipo -archs "$APP/Contents/MacOS/Snipes")
[[ "$ARCHS" == *" "* ]] && ARCH_LABEL=universal || ARCH_LABEL=$ARCHS

if [[ $SIGN == 0 ]]; then
	echo "==> Done (unsigned, $ARCHS): $APP"
	exit 0
fi

echo "==> Signing with: $SIGN_IDENTITY"
for f in "$FW"/*.dylib; do
	codesign --force --timestamp --options runtime --sign "$SIGN_IDENTITY" "$f"
done
codesign --force --timestamp --options runtime --sign "$SIGN_IDENTITY" "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"

ZIP=$DIST/Snipes-$VERSION-macos-$ARCH_LABEL.zip
rm -f "$ZIP"
ditto -c -k --keepParent "$APP" "$ZIP"

if [[ -n "${NOTARY_PROFILE:-}" ]]; then
	echo "==> Notarizing"
	xcrun notarytool submit "$ZIP" --keychain-profile "$NOTARY_PROFILE" --wait
	xcrun stapler staple "$APP"
	rm -f "$ZIP"
	ditto -c -k --keepParent "$APP" "$ZIP"
	spctl --assess --type execute --verbose "$APP"
else
	echo "note: NOTARY_PROFILE not set; skipping notarization (Gatekeeper will warn on other Macs)"
fi

# Disk image: the app on the left, an Applications shortcut on the right, and a
# background with an arrow between them (scripts/dmg-background.swift).
DMG=$DIST/Snipes-$VERSION-macos-$ARCH_LABEL.dmg
if command -v create-dmg >/dev/null; then
	echo "==> Building disk image"
	STAGE=$DIST/dmg-staging
	rm -rf "$STAGE" "$DMG"
	mkdir -p "$STAGE"
	ditto "$APP" "$STAGE/Snipes.app"
	swift scripts/dmg-background.swift "$DIST/dmg-background.png" 1
	swift scripts/dmg-background.swift "$DIST/dmg-background@2x.png" 2
	tiffutil -cathidpicheck "$DIST/dmg-background.png" "$DIST/dmg-background@2x.png" -out "$DIST/dmg-background.tiff" >/dev/null
	VOLICON=$(ls "$APP/Contents/Resources/"*.icns 2>/dev/null | head -1 || true)
	# Icon positions must match those drawn in scripts/dmg-background.swift
	create-dmg \
		--volname "Snipes" \
		${VOLICON:+--volicon "$VOLICON"} \
		--background "$DIST/dmg-background.tiff" \
		--window-pos 200 120 \
		--window-size 640 400 \
		--icon-size 112 \
		--text-size 13 \
		--icon "Snipes.app" 170 185 \
		--hide-extension "Snipes.app" \
		--app-drop-link 470 185 \
		"$DMG" "$STAGE" >/dev/null
	rm -rf "$STAGE" "$DIST"/dmg-background*
	codesign --force --timestamp --sign "$SIGN_IDENTITY" "$DMG"
	if [[ -n "${NOTARY_PROFILE:-}" ]]; then
		echo "==> Notarizing disk image"
		xcrun notarytool submit "$DMG" --keychain-profile "$NOTARY_PROFILE" --wait
		xcrun stapler staple "$DMG"
		spctl --assess --type open --context context:primary-signature --verbose "$DMG"
	fi
else
	echo "note: create-dmg not installed (brew install create-dmg); skipping the disk image"
	DMG=
fi

echo "==> Done: $APP"
echo "         $ZIP"
[[ -n "$DMG" ]] && echo "         $DMG"
true
