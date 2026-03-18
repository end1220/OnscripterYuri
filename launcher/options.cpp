#include "options.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <sys/stat.h>
#include <unistd.h>

static void ensureSaveDir(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return;
    mkdir(path.c_str(), 0777);
}

void stopOtherLauncherProcesses() {
    pid_t self = getpid();
    FILE *fp = popen("pgrep -x onsyuri_launcher 2>/dev/null", "r");
    if (!fp) return;
    char buf[64];
    while (fgets(buf, sizeof(buf), fp)) {
        pid_t pid = static_cast<pid_t>(std::atoi(buf));
        if (pid > 0 && pid != self)
            kill(pid, SIGTERM);
    }
    pclose(fp);
    usleep(200 * 1000);
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
        } else if (arg == "--launcher-data-dir" && i + 1 < argc) {
            opt.launcherDataDir = argv[++i];
        } else if (arg == "--enc" && i + 1 < argc) {
            opt.enc = argv[++i];
        } else if (arg == "--pass-arg" && i + 1 < argc) {
            opt.passArgs.emplace_back(argv[++i]);
        }
    }

    if (opt.onsyuriPath.empty() || opt.gamesRoot.empty()) {
        std::fprintf(stderr,
                     "Usage: launcher --onsyuri /path/to/onsyuri --games-root /path/to/ONS "
                     "[--launcher-data-dir /path/to/ONScripter] "
                     "[--font /path/to/font.ttf] "
                     "[--enc utf8] [--pass-arg ARG]...\n");
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

    // 直接启动 onsyuri 的参数（不传 enc）
    std::vector<std::string> argStorage;
    argStorage.push_back(opt.onsyuriPath);
    argStorage.push_back("--root");
    argStorage.push_back(game.path);
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

    execv(opt.onsyuriPath.c_str(), args.data());
    std::perror("execv onsyuri");
    std::fprintf(stderr, "[Launcher] launchGame: FAILED execv onsyuri\n");
    return 1;
}
