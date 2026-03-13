#include "options.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

static void ensureSaveDir(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return;
    mkdir(path.c_str(), 0777);
}

bool parseArgs(int argc, char *argv[], Options &opt) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--onsyuri" && i + 1 < argc) {
            opt.onsyuriPath = argv[++i];
        } else if (arg == "--games-root" && i + 1 < argc) {
            opt.gamesRoot = argv[++i];
        } else if (arg == "--font" && i + 1 < argc) {
            opt.fontPath = argv[++i];
        } else if (arg == "--enc" && i + 1 < argc) {
            opt.enc = argv[++i];
        } else if (arg == "--pass-arg" && i + 1 < argc) {
            opt.passArgs.emplace_back(argv[++i]);
        }
    }

    if (opt.onsyuriPath.empty() || opt.gamesRoot.empty()) {
        std::fprintf(stderr,
                     "Usage: launcher --onsyuri /path/to/onsyuri --games-root /path/to/ONS "
                     "[--font font.ttf] [--enc utf8] [--pass-arg ARG]...\n");
        return false;
    }
    return true;
}

int launchGame(const Options &opt, const GameEntry &game) {
    std::string saveDir = game.path + "save";
    ensureSaveDir(saveDir);

    std::string gameFont = game.path + "default.ttf";  // 每个游戏用自己目录下的 default.ttf

    std::vector<std::string> argStorage;
    argStorage.push_back(opt.onsyuriPath);
    argStorage.push_back("--root");
    argStorage.push_back(game.path);
    argStorage.push_back("--enc:" + opt.enc);
    argStorage.push_back("--font");
    argStorage.push_back(gameFont);
    argStorage.push_back("--save-dir");
    argStorage.push_back(saveDir);
    for (const auto &p : opt.passArgs)
        argStorage.push_back(p);

    std::vector<char *> args;
    args.reserve(argStorage.size() + 1);
    for (auto &s : argStorage)
        args.push_back(&s[0]);
    args.push_back(nullptr);

    execv(opt.onsyuriPath.c_str(), args.data());
    std::perror("execv");
    return 1;
}
