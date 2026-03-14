#!/bin/bash
# 参数化 ONSYURI 启动脚本（供 onsyuri_launcher 调用）

set -u

ONSYURI=""
GAME_DIR=""
SAVE_DIR=""
FONT=""
ENC="utf8"
PASS_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --onsyuri)
            ONSYURI="${2:-}"
            shift 2
            ;;
        --game-dir)
            GAME_DIR="${2:-}"
            shift 2
            ;;
        --save-dir)
            SAVE_DIR="${2:-}"
            shift 2
            ;;
        --font)
            FONT="${2:-}"
            shift 2
            ;;
        --enc)
            ENC="${2:-utf8}"
            shift 2
            ;;
        --pass-arg)
            PASS_ARGS+=("${2:-}")
            shift 2
            ;;
        *)
            echo "Unknown arg: $1" >&2
            shift
            ;;
    esac
done

if [[ -z "$ONSYURI" || -z "$GAME_DIR" ]]; then
    echo "Usage: $0 --onsyuri /path/to/onsyuri --game-dir /path/to/game [--save-dir DIR] [--font FILE] [--enc utf8] [--pass-arg ARG]..." >&2
    exit 1
fi

if [[ -z "$SAVE_DIR" ]]; then
    SAVE_DIR="$GAME_DIR/save"
fi
mkdir -p "$SAVE_DIR"

if [[ -z "$FONT" ]]; then
    FONT="$GAME_DIR/default.ttf"
fi

if [[ ! -x "$ONSYURI" ]]; then
    echo "Error: cannot find executable $ONSYURI" >&2
    exit 1
fi

if [[ ! -d "$GAME_DIR" ]]; then
    echo "Error: game directory does not exist $GAME_DIR" >&2
    exit 1
fi

ONS_DIR="$(cd "$(dirname "$ONSYURI")" && pwd)"
cd "$ONS_DIR" || exit 1

export HOME=/root
export LD_LIBRARY_PATH="$ONS_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

LOG_FILE="$ONS_DIR/log.txt"

EXTRA_ARGS=(--save-dir "$SAVE_DIR")
if [[ -f "$FONT" ]]; then
    EXTRA_ARGS+=(--font "$FONT")
fi

"$ONSYURI" --root "$GAME_DIR" "${EXTRA_ARGS[@]}" "${PASS_ARGS[@]}" >"$LOG_FILE" 2>&1
exit $?

