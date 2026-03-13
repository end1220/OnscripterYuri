#include "game_list.h"
#include "menu_ui.h"
#include "options.h"

#include <cstdio>

int main(int argc, char *argv[]) {
    Options opt;
    if (!parseArgs(argc, argv, opt)) return 1;

    auto games = scanGames(opt.gamesRoot);
    if (games.empty()) {
        std::fprintf(stderr, "No games found under %s\n", opt.gamesRoot.c_str());
        return 1;
    }

    MenuUI ui;
    if (!ui.init(opt.fontPath)) return 1;

    int selected = ui.run(games);
    ui.shutdown();

    if (selected >= 0 && selected < static_cast<int>(games.size()))
        return launchGame(opt, games[selected]);

    return 0;
}
