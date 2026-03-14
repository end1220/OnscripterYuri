#ifndef LAUNCHER_GAME_LIST_H
#define LAUNCHER_GAME_LIST_H

#include <string>
#include <vector>

struct GameEntry {
    std::string name;      // 子文件夹名，作为游戏显示名
    std::string path;      // 游戏根目录路径
    std::string iconPath;  // icon.png 完整路径，空表示无图标
};

std::vector<GameEntry> scanGames(const std::string &root);

#endif
