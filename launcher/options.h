#ifndef LAUNCHER_OPTIONS_H
#define LAUNCHER_OPTIONS_H

#include "game_list.h"

#include <string>
#include <vector>

struct Options {
    std::string onsyuriPath;
    std::string gamesRoot;
    std::string fontPath;
    std::string launcherDataDir;  // data.pak 所在目录，如 /path/to/ONScripter
    std::string enc = "utf8";
    std::vector<std::string> passArgs;
    bool windowed = false;  // 窗口模式（用于开发调试）
    int windowWidth = 960;   // 窗口模式下的宽度
    int windowHeight = 720;  // 窗口模式下的高度
};

bool parseArgs(int argc, char *argv[], Options &opt);

int launchGame(const Options &opt, const GameEntry &game);

/** 启动时关闭其他 ons_launcher 进程，保证单实例。 */
void stopOtherLauncherProcesses();

#endif
