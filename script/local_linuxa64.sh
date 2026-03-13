# bash -c "BUILD_TYPE=Debug OBTAIN_DEPENDS=yes ./local_linuxa64.sh"
# 在 Ubuntu ARM64 上本地构建，使用系统库
PLATFORM=linuxa64
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo 2>/dev/null | grep -c ^processor || nproc)
TARGETS=$@

# config env
CC=gcc
CXX=g++
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# ports from debian
if [ -n "$OBTAIN_DEPENDS" ]; then
    sudo apt-get update
    sudo apt-get -y install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
    sudo apt-get -y install libbz2-dev libjpeg-dev libpng-dev
    sudo apt-get -y install liblua5.3-dev libgl1-mesa-dev
fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE"
echo "PLATFORM=$PLATFORM (native ARM64)"

cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX

make -C $BUILD_PATH $TARGETS -j$CORE_NUM

# 构建完成后，将依赖的共享库拷贝到输出目录的 lib 文件夹（已关闭，需要时可设 COPY_DEPS=yes 启用）
# if [ -x "$BUILD_PATH/onsyuri" ]; then
#     mkdir -p "$BUILD_PATH/lib"
#     echo "Copying dependent libraries to $BUILD_PATH/lib ..."
#     ldd "$BUILD_PATH/onsyuri" 2>/dev/null | grep "=>" | awk '{print $3}' | while read -r so; do
#         if [ -n "$so" ] && [ -f "$so" ]; then
#             case "$(basename "$so")" in
#                 libc.so*|libm.so*|libdl.so*|libpthread.so*|ld-linux*.so*)
#                     ;;
#                 *)
#                     cp -L -n "$so" "$BUILD_PATH/lib/" 2>/dev/null && echo "  $(basename "$so")" || true
#                     ;;
#             esac
#         fi
#     done
#     echo "Done."
# fi
