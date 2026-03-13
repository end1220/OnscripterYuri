#ifndef LAUNCHER_GAME_LIST_H
#define LAUNCHER_GAME_LIST_H

#include <string>
#include <vector>

struct GameEntry {
    std::string name;
    std::string path;
};

std::vector<GameEntry> scanGames(const std::string &root);

#endif
