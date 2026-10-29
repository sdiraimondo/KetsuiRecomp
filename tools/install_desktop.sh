#!/usr/bin/env bash
# Installs a desktop launcher for the built Ketsui binary, in the user's home only.
# Writes: ~/.local/share/applications/ketsui.desktop
#         ~/.local/share/icons/hicolor/scalable/apps/ketsui.svg
# Nothing from the game is copied. Usage: tools/install_desktop.sh
set -eu
PROJECT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$PROJECT/build/ketsui/ketsui"
if [ ! -x "$BIN" ]; then
    echo "binary not found: $BIN (build first)" >&2
    exit 1
fi

APPS="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICONS="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/scalable/apps"
mkdir -p "$APPS" "$ICONS"
cp "$PROJECT/packaging/ketsui.svg" "$ICONS/ketsui.svg"

cat > "$APPS/ketsui.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Ketsui
Comment=Ketsui (Xbox 360) recompiled with ReXGlue
Exec=$BIN
Path=$PROJECT/build/ketsui
Icon=ketsui
Terminal=false
Categories=Game;
EOF

echo "installed: $APPS/ketsui.desktop"
echo "installed: $ICONS/ketsui.svg"
