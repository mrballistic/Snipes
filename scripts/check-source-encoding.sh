#!/usr/bin/env bash
# Fails if a C/C++ source under src/ contains non-ASCII text but no UTF-8 byte
# order mark. Visual Studio reads BOM-less files in the system code page, which
# garbles strings such as "←", "☺", "·" and "©" in the Windows build.
status=0
while IFS= read -r -d '' f; do
	if LC_ALL=C grep -q '[^[:print:][:space:]]' "$f" && [[ "$(head -c 3 "$f" | xxd -p)" != "efbbbf" ]]; then
		echo "error: $f has non-ASCII text but no UTF-8 BOM" >&2
		status=1
	fi
done < <(find src \( -name '*.cpp' -o -name '*.h' \) -print0)
exit $status
