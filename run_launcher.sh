#!/bin/bash
# ONS 游戏列表启动器 - 掌机

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ONS_DIR="$SCRIPT_DIR/ONScripter"
GAME_ROOT="${GAME_ROOT:-$SCRIPT_DIR/../ONS}"

cd "$ONS_DIR" || exit 1
export LD_LIBRARY_PATH="$SCRIPT_DIR/ONScripter/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

"$ONS_DIR/onsyuri_launcher" \
    --onsyuri "$ONS_DIR/onsyuri" \
    --games-root "$GAME_ROOT" \
    --launch-script "$ONS_DIR/run_game.sh" \
    --font "$ONS_DIR/SYHT-Normal.otf" \
    --pass-arg "--fullscreen2" --pass-arg "--sharpness" --pass-arg "3.1" \
    "$@" >"$ONS_DIR/log_launcher.txt" 2>&1
