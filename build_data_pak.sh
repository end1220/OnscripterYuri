#!/bin/bash
# 打包 launcher 资源为 data.pak
# 用法: ./build_data_pak.sh [assets路径] [data.pak输出路径]
# 默认: assets=launcher/assets, 输出=build_linuxa64/launcher/data.pak

set -e
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

ASSETS="${1:-launcher/assets}"
PAK_PATH="${2:-build_linuxa64/launcher/data.pak}"
BUILDER="build_linuxa64/launcher/onsyuri_pak_builder"

if [[ ! -d "$ASSETS" ]]; then
    echo "错误: assets 目录不存在: $ASSETS" >&2
    exit 1
fi
if [[ ! -x "$BUILDER" ]]; then
    echo "错误: 请先编译 launcher，生成 $BUILDER" >&2
    exit 1
fi

mkdir -p "$(dirname "$PAK_PATH")"
"$BUILDER" "$ASSETS" "$PAK_PATH"
echo "已生成: $PAK_PATH"
