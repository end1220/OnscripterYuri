#include "options.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

static void ensureSaveDir(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return;
    mkdir(path.c_str(), 0777);
}

static void stopExistingOnsyuriProcesses() {
    // 先清理残留 onsyuri 进程，避免输入设备被旧进程占用
    int rc = std::system("pkill -x onsyuri >/dev/null 2>&1");
    (void)rc;
    usleep(200 * 1000);  // 给系统一点时间回收资源
}

static void clearLauncherSdlEnv() {
    // launcher 为兜底可能设置过 SDL_VIDEODRIVER，启动游戏前清掉
    unsetenv("SDL_VIDEODRIVER");
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
        } else if (arg == "--launch-script" && i + 1 < argc) {
            opt.launchScriptPath = argv[++i];
        } else if (arg == "--enc" && i + 1 < argc) {
            opt.enc = argv[++i];
        } else if (arg == "--pass-arg" && i + 1 < argc) {
            opt.passArgs.emplace_back(argv[++i]);
        }
    }

    if (opt.onsyuriPath.empty() || opt.gamesRoot.empty()) {
        std::fprintf(stderr,
                     "Usage: launcher --onsyuri /path/to/onsyuri --games-root /path/to/ONS "
                     "[--launch-script /path/to/script.sh] "
                     "[--font font.ttf] [--enc utf8] [--pass-arg ARG]...\n");
        return false;
    }
    return true;
}

int launchGame(const Options &opt, const GameEntry &game) {
    std::fprintf(stderr, "[Launcher] launchGame: begin game=%s path=%s\n",
                 game.name.c_str(), game.path.c_str());
    stopExistingOnsyuriProcesses();
    std::fprintf(stderr, "[Launcher] launchGame: existing onsyuri cleaned\n");
    clearLauncherSdlEnv();
    std::fprintf(stderr, "[Launcher] launchGame: SDL_VIDEODRIVER=%s\n",
                 getenv("SDL_VIDEODRIVER") ? getenv("SDL_VIDEODRIVER") : "(null)");

    std::string saveDir = game.path + "save";
    ensureSaveDir(saveDir);

    std::string gameFont = game.path + "default.ttf";  // 每个游戏用自己目录下的 default.ttf
    std::fprintf(stderr, "[Launcher] launchGame: saveDir=%s font=%s\n",
                 saveDir.c_str(), gameFont.c_str());

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
    std::fprintf(stderr, "[Launcher] launchGame: direct exec args count=%zu\n", argStorage.size());
    for (std::size_t i = 0; i < argStorage.size(); ++i)
        std::fprintf(stderr, "[Launcher] launchGame: arg[%zu]=%s\n", i, argStorage[i].c_str());

    std::vector<char *> args;
    args.reserve(argStorage.size() + 1);
    for (auto &s : argStorage)
        args.push_back(&s[0]);
    args.push_back(nullptr);

    if (!opt.launchScriptPath.empty()) {
        std::fprintf(stderr, "[Launcher] launchGame: using script=%s\n", opt.launchScriptPath.c_str());
        std::vector<std::string> scriptArgStorage;
        scriptArgStorage.push_back("/bin/bash");
        scriptArgStorage.push_back(opt.launchScriptPath);
        scriptArgStorage.push_back("--onsyuri");
        scriptArgStorage.push_back(opt.onsyuriPath);
        scriptArgStorage.push_back("--game-dir");
        scriptArgStorage.push_back(game.path);
        scriptArgStorage.push_back("--enc");
        scriptArgStorage.push_back(opt.enc);
        scriptArgStorage.push_back("--save-dir");
        scriptArgStorage.push_back(saveDir);
        scriptArgStorage.push_back("--font");
        scriptArgStorage.push_back(gameFont);
        for (const auto &p : opt.passArgs) {
            scriptArgStorage.push_back("--pass-arg");
            scriptArgStorage.push_back(p);
        }
        std::fprintf(stderr, "[Launcher] launchGame: script args count=%zu\n", scriptArgStorage.size());
        for (std::size_t i = 0; i < scriptArgStorage.size(); ++i)
            std::fprintf(stderr, "[Launcher] launchGame: scriptArg[%zu]=%s\n",
                         i, scriptArgStorage[i].c_str());

        std::vector<char *> scriptArgs;
        scriptArgs.reserve(scriptArgStorage.size() + 1);
        for (auto &s : scriptArgStorage)
            scriptArgs.push_back(&s[0]);
        scriptArgs.push_back(nullptr);

        execv("/bin/bash", scriptArgs.data());
        std::perror("execv /bin/bash");
        std::fprintf(stderr, "[Launcher] launchGame: FAILED execv /bin/bash\n");
        return 1;
    }

    // 兜底：未提供脚本时直接启动 onsyuri
    execv(opt.onsyuriPath.c_str(), args.data());
    std::perror("execv onsyuri");
    std::fprintf(stderr, "[Launcher] launchGame: FAILED execv onsyuri\n");
    return 1;
}
