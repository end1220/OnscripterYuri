#include "game_list.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

static bool isDirectory(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

static bool isRegularFile(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

/** ONS 脚本入口（与 ScriptHandler::readScript 一致） */
static bool hasOnsScriptEntry(const std::string &dirPath) {
    static const char *const kScriptEntries[] = {
        "0.txt",
        "00.txt",
        "nscr_sec.dat",
        "nscript.___",
        "nscript.dat",
        "onscript.nt2",
        "onscript.nt3",
    };
    for (const char *name : kScriptEntries) {
        if (isRegularFile(dirPath + "/" + name))
            return true;
    }
    return false;
}

/** 常见资源包后缀（arc.sar 为固定名，另扫 .nsa / .ns2） */
static bool hasOnsArchiveFile(const std::string &dirPath) {
    if (isRegularFile(dirPath + "/arc.sar"))
        return true;

    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return false;
    bool found = false;
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        std::string name = ent->d_name;
        const size_t len = name.size();
        if (len >= 4) {
            if (name.compare(len - 4, 4, ".nsa") == 0 ||
                name.compare(len - 4, 4, ".ns2") == 0) {
                if (isRegularFile(dirPath + "/" + name)) {
                    found = true;
                    break;
                }
            }
        }
    }
    closedir(dir);
    return found;
}

/** 目录是否为可运行的 ONS 游戏根（脚本入口优先，与引擎判定一致） */
static bool isOnsGameDir(const std::string &dirPath) {
    return hasOnsScriptEntry(dirPath) || hasOnsArchiveFile(dirPath);
}

static std::string basenameOf(std::string path) {
    while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
        path.pop_back();
    const size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
        return path;
    return path.substr(pos + 1);
}

static std::string withTrailingSlash(const std::string &path) {
    if (path.empty() || path.back() == '/')
        return path;
    return path + "/";
}

/** 按优先级查找游戏图标：icon.png -> logo.png -> 子目录内任意 .png 文件 */
static std::string findGameIcon(const std::string &dirPath) {
    if (isRegularFile(dirPath + "/icon.png"))
        return dirPath + "/icon.png";
    if (isRegularFile(dirPath + "/logo.png"))
        return dirPath + "/logo.png";
    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return "";
    std::string found;
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        std::string name = ent->d_name;
        size_t len = name.size();
        if (len < 4) continue;
        if (name.compare(len - 4, 4, ".png") == 0) {
            std::string full = dirPath + "/" + name;
            if (isRegularFile(full)) {
                found = full;
                break;
            }
        }
    }
    closedir(dir);
    return found;
}

std::vector<GameEntry> scanGames(const std::string &root) {
    std::vector<GameEntry> games;

    // games-root 本身即为游戏目录（文件直接在 ONS 根下，无子文件夹）
    if (isOnsGameDir(root)) {
        std::string gamePath = withTrailingSlash(root);
        games.push_back(GameEntry{basenameOf(root), gamePath, findGameIcon(root)});
    }

    DIR *dir = opendir(root.c_str());
    if (!dir) {
        std::sort(games.begin(), games.end(), [](const GameEntry &a, const GameEntry &b) {
            return a.name < b.name;
        });
        return games;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        std::string sub = root + "/" + ent->d_name;
        if (!isDirectory(sub)) continue;

        if (!isOnsGameDir(sub)) continue;

        std::string name = ent->d_name;
        std::string iconPath = findGameIcon(sub);
        games.push_back(GameEntry{name, withTrailingSlash(sub), iconPath});
    }
    closedir(dir);

    std::sort(games.begin(), games.end(), [](const GameEntry &a, const GameEntry &b) {
        return a.name < b.name;
    });
    return games;
}
