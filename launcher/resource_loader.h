#ifndef LAUNCHER_RESOURCE_LOADER_H
#define LAUNCHER_RESOURCE_LOADER_H

#include "resource_pak.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// ResourceLoader 封装了对 data.pak 的访问，并提供从 pak 中
// 直接加载 SDL_Texture / TTF_Font 的便捷接口。
//
// 设计要点：
// - 仅负责从 pak 读取数据并创建 SDL 资源，不做缓存；
// - SDL 资源（Texture/Font）的生命期仍由调用者管理（负责销毁）；
// - 如果 data.pak 不存在或某个资源缺失，接口返回 nullptr，调用方可以自行回退到文件系统加载。

class ResourceLoader {
public:
    static ResourceLoader &instance();

    // 尝试打开指定 pak 文件（默认 "data.pak"）
    bool init(const std::string &pakPath = "data.pak");
    void shutdown();

    bool isReady() const { return pak_.isOpen(); }

    // 从 pak 中读取原始字节到 outData。
    bool loadBytes(const std::string &name, std::vector<std::uint8_t> &outData) const;

    // 从 pak 加载纹理。成功返回 SDL_Texture*，失败返回 nullptr。
    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &name) const;

    // 从 pak 加载字体。成功返回 TTF_Font*，失败返回 nullptr。
    TTF_Font *loadFont(const std::string &name, int ptSize);

private:
    ResourceLoader() = default;
    ~ResourceLoader() = default;

    ResourceLoader(const ResourceLoader &) = delete;
    ResourceLoader &operator=(const ResourceLoader &) = delete;

    struct FontCacheEntry {
        std::vector<std::uint8_t> data; // 必须常驻，避免 SDL_ttf 延迟读取时访问悬空内存
        SDL_RWops *rw = nullptr;        // 对应 data 的 RWops
        TTF_Font *font = nullptr;       // 打开的字体
    };

    static std::string makeFontKey(const std::string &name, int ptSize);

    ResourcePak pak_;
    std::unordered_map<std::string, FontCacheEntry> fontCache_;
};

#endif // LAUNCHER_RESOURCE_LOADER_H

