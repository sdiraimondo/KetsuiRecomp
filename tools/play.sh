#!/usr/bin/env bash
# Scripted play-through for milestone tests (keyboard as controller via xdotool).
# Usage: tools/play.sh <run_name> <seconds>
# Layers Vulkan off (rule 8). Log goes to logs/<run_name>.log.
set -u
RUN="${1:?run name}"
SECS="${2:-180}"
PROJECT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT" || exit 1

export VK_LOADER_LAYERS_DISABLE='*' MANGOHUD=0 DISABLE_LS_FRAME_GENERATION=1
export LD_LIBRARY_PATH="$PROJECT/rexglue-sdk/out/linux-amd64"

timeout -s INT $((SECS + 60)) ./build/ketsui/ketsui \
    --game_data_root "$PROJECT/game_files" \
    --gpu_plugin xenos \
    --mnk_mode \
    --log_file "$PROJECT/logs/$RUN.log" \
    > "logs/${RUN}_stdout.txt" 2>&1 &
GAME=$!

WID=""
for _ in $(seq 1 60); do
    WID=$(xdotool search --name '^ketsui \[' 2>/dev/null | head -1)
    [ -n "$WID" ] && break
    sleep 1
done
if [ -z "$WID" ]; then
    echo "no game window found"; kill -INT "$GAME" 2>/dev/null; wait "$GAME"; exit 2
fi
xdotool windowactivate --sync "$WID" 2>/dev/null || xdotool windowfocus "$WID"
echo "window $WID, playing ${SECS}s"

# Hold each press: the mnk driver polls key state, a bare tap can be missed.
press() { xdotool keydown "$1"; sleep 0.3; xdotool keyup "$1"; }
for t in 12 20 28; do
    sleep 8
    press Return            # Start (keybind_start: X,Return), presses spread out
    echo "start press at ~${t}s"
done

START=$(date +%s)
DIR=0
while [ $(( $(date +%s) - START )) -lt "$SECS" ]; do
    if ! kill -0 "$GAME" 2>/dev/null; then
        echo "game exited early"; break
    fi
    xdotool keydown space    # A = fire (keybind_a: Semicolon,Space)
    case $DIR in
        0) xdotool keydown a; sleep 1.5; xdotool keyup a ;;
        1) xdotool keydown d; sleep 1.5; xdotool keyup d ;;
        2) xdotool keydown w; sleep 1; xdotool keyup w; xdotool keydown s; sleep 0.8; xdotool keyup s ;;
    esac
    DIR=$(( (DIR + 1) % 3 ))
    sleep 0.5
done
xdotool keyup space 2>/dev/null

kill -INT "$GAME" 2>/dev/null
wait "$GAME"
echo "game exit=$?  (124 = stopped by timer, expected)"
