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

/** 检查子目录中是否存在 .nsa 后缀的文件 */
static bool hasNsaFile(const std::string &dirPath) {
    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return false;
    bool found = false;
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        std::string name = ent->d_name;
        size_t len = name.size();
        if (len < 4) continue;
        if (name.compare(len - 4, 4, ".nsa") == 0) {
            std::string full = dirPath + "/" + name;
            if (isRegularFile(full)) {
                found = true;
                break;
            }
        }
    }
    closedir(dir);
    return found;
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
    DIR *dir = opendir(root.c_str());
    if (!dir) return games;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        std::string sub = root + "/" + ent->d_name;
        if (!isDirectory(sub)) continue;

        if (!hasNsaFile(sub)) continue;

        std::string name = ent->d_name;
        std::string iconPath = findGameIcon(sub);
        games.push_back(GameEntry{name, sub + "/", iconPath});
    }
    closedir(dir);

    std::sort(games.begin(), games.end(), [](const GameEntry &a, const GameEntry &b) {
        return a.name < b.name;
    });
    return games;
}
