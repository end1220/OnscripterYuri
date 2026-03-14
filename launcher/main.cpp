#include "game_list.h"
#include "menu_ui.h"
#include "options.h"

#include <cstdio>

int main(int argc, char *argv[]) {
    std::fprintf(stderr, "[Launcher] main: start argc=%d\n", argc);
    Options opt;
    if (!parseArgs(argc, argv, opt)) return 1;
    std::fprintf(stderr, "[Launcher] main: parsed onsyuri=%s gamesRoot=%s launchScript=%s enc=%s passArgs=%zu\n",
                 opt.onsyuriPath.c_str(),
                 opt.gamesRoot.c_str(),
                 opt.launchScriptPath.empty() ? "(none)" : opt.launchScriptPath.c_str(),
                 opt.enc.c_str(),
                 opt.passArgs.size());

    auto games = scanGames(opt.gamesRoot);
    std::fprintf(stderr, "[Launcher] main: scanned games count=%zu\n", games.size());
    if (games.empty()) {
        std::fprintf(stderr, "No games found under %s\n", opt.gamesRoot.c_str());
        return 1;
    }

    MenuUI ui;
    if (!ui.init(opt.fontPath)) return 1;

    int selected = ui.run(games);
    std::fprintf(stderr, "[Launcher] main: ui.run returned selected=%d\n", selected);
    ui.shutdown();
    std::fprintf(stderr, "[Launcher] main: ui.shutdown done\n");

    if (selected >= 0 && selected < static_cast<int>(games.size())) {
        std::fprintf(stderr, "[Launcher] main: launching game idx=%d name=%s path=%s\n",
                     selected, games[selected].name.c_str(), games[selected].path.c_str());
        return launchGame(opt, games[selected]);
    }

    std::fprintf(stderr, "[Launcher] main: exit without launching game\n");
    return 0;
}
