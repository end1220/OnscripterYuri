#!/bin/bash
# 本机开发机测试脚本（窗口模式 960x720）

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ONSYURI="$SCRIPT_DIR/build_linuxa64/onscripter"
#GAME_DIR="$SCRIPT_DIR/build_linuxa64/ONS/勇者大战魔物娘"
GAME_DIR="$SCRIPT_DIR/build_linuxa64/ONS/秋之回忆2"
SAVE_DIR="$GAME_DIR/save"
FONT="$GAME_DIR/default.ttf"

mkdir -p "$SAVE_DIR"

if [ ! -x "$ONSYURI" ]; then
    echo "Error: cannot find executable file: $ONSYURI"
    exit 1
fi

if [ ! -d "$GAME_DIR" ]; then
    echo "Error: game directory does not exist: $GAME_DIR"
    exit 1
fi

LOG_FILE="$SCRIPT_DIR/build_linuxa64/log_test_onsyuri.txt"

EXTRA_ARGS=()
[ -n "$SAVE_DIR" ] && EXTRA_ARGS+=(--save-dir "$SAVE_DIR")
[ -n "$FONT" ] && [ -f "$FONT" ] && EXTRA_ARGS+=(--font "$FONT")

cd "$(dirname "$ONSYURI")"
"$ONSYURI" \
    --root "$GAME_DIR" \
    --window \
    --width 960 \
    --height 720 \
    "${EXTRA_ARGS[@]}" \
    "$@" > "$LOG_FILE" 2>&1
