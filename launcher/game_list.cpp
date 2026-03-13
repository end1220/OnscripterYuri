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

static std::string readTitleFromFile(const std::string &path) {
    FILE *fp = std::fopen(path.c_str(), "r");
    if (!fp) return "";
    char buf[256];
    std::string title;
    if (std::fgets(buf, sizeof(buf), fp)) {
        size_t len = std::strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';
        if (len > 0) title = buf;
    }
    std::fclose(fp);
    return title;
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

        std::string title = ent->d_name;
        std::string fromFile = readTitleFromFile(sub + "/title.txt");
        if (!fromFile.empty()) title = fromFile;

        games.push_back(GameEntry{title, sub + "/"});
    }
    closedir(dir);

    std::sort(games.begin(), games.end(), [](const GameEntry &a, const GameEntry &b) {
        return a.name < b.name;
    });
    return games;
}
