#!/bin/bash
# Onscripter Yuri 启动脚本 - Ubuntu ARM64 掌机
# 以本脚本所在目录为参照配置路径

# 终止已运行的 onsyuri 进程
# pkill -x onsyuri 2>/dev/null
# sleep 0.5

export HOME=/root
echo "=== Starting onsyuri ===" > /dev/tty0

# ========== 路径配置（相对于本脚本所在目录）==========
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ONSYURI="$SCRIPT_DIR/ONScripter/onsyuri"

GAME_DIR="$SCRIPT_DIR/../ONS/yongzhe"

SAVE_DIR="$GAME_DIR/save"
[ -n "$SAVE_DIR" ] && mkdir -p "$SAVE_DIR"

FONT="$GAME_DIR/default.ttf"

# 脚本编码：utf8 / gbk / sjis
ENC="utf8"

# ========== 启动参数 ==========
# 全屏拉伸模式（F10 切换），适配掌机屏幕
# --fullscreen2 全屏拉伸
# --sharpness 锐化（可选，如 3.1）
OPTS="--fullscreen2 --sharpness 3.1"

# ========== 启动 ==========
if [ ! -x "$ONSYURI" ]; then
    echo "Error: cannot find executable file $ONSYURI"
    echo "Please check the ONSYURI path, or build it first"
    exit 1
fi

if [ ! -d "$GAME_DIR" ]; then
    echo "Error: game directory does not exist $GAME_DIR"
    echo "Please put the game files into this directory, or modify the GAME_DIR variable"
fi

cd "$(dirname "$ONSYURI")" || exit 1

export LD_LIBRARY_PATH="${SCRIPT_DIR}/ONScripter/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

LOG_DIR="$(dirname "$ONSYURI")"
LOG_FILE="$LOG_DIR/log.txt"

EXTRA_ARGS=()
[ -n "$SAVE_DIR" ] && EXTRA_ARGS+=(--save-dir "$SAVE_DIR")
[ -n "$FONT" ] && [ -f "$FONT" ] && EXTRA_ARGS+=(--font "$FONT")

"$ONSYURI" --root "$GAME_DIR" --enc:"$ENC" "${EXTRA_ARGS[@]}" $OPTS "$@" > "$LOG_FILE" 2>&1

echo "onsyuri completed" > /dev/tty0
