#!/bin/bash
# WSL开发机上测试启动器 - Linux ARM64。开发机是wsl+xlaunch用于模拟运行，区别于在实际掌机上运行。

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ONS_DIR="$SCRIPT_DIR/build_linuxa64"
LAUNCHER_DIR="$ONS_DIR/launcher"
GAME_ROOT="${GAME_ROOT:-$SCRIPT_DIR/../ONS}"

cd "$ONS_DIR" || exit 1
export LD_LIBRARY_PATH="$ONS_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# 打包 launcher 资源（如果需要）
if [ ! -f "$LAUNCHER_DIR/data.pak" ] || [ "$SCRIPT_DIR/launcher/assets" -nt "$LAUNCHER_DIR/data.pak" ]; then
    if [ -d "$SCRIPT_DIR/launcher/assets" ]; then
        cp -r "$SCRIPT_DIR/launcher/assets" "$LAUNCHER_DIR/"
        cd "$LAUNCHER_DIR" || exit 1
        ./ons_pak_builder
        cd "$ONS_DIR" || exit 1
    fi
fi

"$LAUNCHER_DIR/ons_launcher" \
    --onscripter "$ONS_DIR/onscripter" \
    --games-root "$GAME_ROOT" \
    --font "$LAUNCHER_DIR/assets/font.ttf" \
    --launcher-data-dir "$LAUNCHER_DIR" \
    --windowed 960 720 \
    --pass-arg "--fullscreen2" --pass-arg "--sharpness" --pass-arg "9.1" \
    "$@" >"$ONS_DIR/log0.txt" 2>&1
