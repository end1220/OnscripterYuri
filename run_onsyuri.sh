#!/bin/bash
# Onscripter Yuri 启动脚本 - Ubuntu ARM64 掌机
# 以本脚本所在目录为参照配置路径

# ========== 路径配置（相对于本脚本所在目录）==========
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# onsyuri 可执行文件路径
ONSYURI="$SCRIPT_DIR/ONScripter/onsyuri"

# 游戏目录（游戏资源所在路径）
GAME_DIR="$SCRIPT_DIR/../ONS/yongzhe"

# 存档目录（留空则使用默认位置）
SAVE_DIR="$GAME_DIR/save"
[ -n "$SAVE_DIR" ] && mkdir -p "$SAVE_DIR"

# 字体文件（可选，留空使用内置或游戏自带）
# 如: FONT="$GAME_DIR/default.ttf"
FONT="$GAME_DIR/default.ttf"

# 脚本编码：utf8 / gbk / sjis
ENC="utf8"

# ========== 启动参数 ==========
# 全屏拉伸模式（F10 切换），适配掌机屏幕
# --fullscreen2 全屏拉伸
# --sharpness 锐化（可选，如 3.1）
OPTS="--fullscreen2 --sharpness 3.1 --width 960 --height 720"

# SDL2 默认支持摇杆/手柄，无需额外参数
# 长按可唤出菜单（4指/长按触摸）

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

cd "$SCRIPT_DIR"

export SDL_VIDEODRIVER_DEBUG=1
export SDL_VIDEO_DEBUG=1

# 优先使用 ONSYURI 同目录下的 lib 中的库
export LD_LIBRARY_PATH="${SCRIPT_DIR}/ONScripter/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# 日志输出到 ONSYURI 所在目录的 log.txt
LOG_DIR="$(dirname "$ONSYURI")"
LOG_FILE="$LOG_DIR/log.txt"

# 构建启动参数
EXTRA_ARGS=()
[ -n "$SAVE_DIR" ] && EXTRA_ARGS+=(--save-dir "$SAVE_DIR")
[ -n "$FONT" ] && [ -f "$FONT" ] && EXTRA_ARGS+=(--font "$FONT")

# 执行，stdout/stderr 输出到 log.txt
exec "$ONSYURI" --root "$GAME_DIR" --enc:"$ENC" "${EXTRA_ARGS[@]}" $OPTS "$@" > "$LOG_FILE" 2>&1
