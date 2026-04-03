#include "game_list.h"
#include "menu_ui.h"
#include "options.h"

#include <cstdio>

int main(int argc, char *argv[]) {
    stopOtherLauncherProcesses();
    Options opt;
    if (!parseArgs(argc, argv, opt)) return 1;

    auto games = scanGames(opt.gamesRoot);

    MenuUI ui;
    if (!ui.init(opt.fontPath, opt.launcherDataDir, opt.windowed, opt.windowWidth, opt.windowHeight)) return 1;

    int selected = ui.run(games);
    ui.shutdown();

    if (selected >= 0 && selected < static_cast<int>(games.size())) {
        return launchGame(opt, games[selected]);
    }

    return 0;
}
