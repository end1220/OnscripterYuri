#ifndef LAUNCHER_OPTIONS_H
#define LAUNCHER_OPTIONS_H

#include "game_list.h"

#include <string>
#include <vector>

struct Options {
    std::string onsyuriPath;
    std::string gamesRoot;
    std::string fontPath;
    std::string launchScriptPath;
    std::string enc = "utf8";
    std::vector<std::string> passArgs;
};

bool parseArgs(int argc, char *argv[], Options &opt);

int launchGame(const Options &opt, const GameEntry &game);

#endif
