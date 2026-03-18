#include "resource_loader.h"

#include <SDL_image.h>

#include <cstdio>

ResourceLoader &ResourceLoader::instance() {
    static ResourceLoader inst;
    return inst;
}

std::string ResourceLoader::makeFontKey(const std::string &name, int ptSize) {
    return name + "#" + std::to_string(ptSize);
}

bool ResourceLoader::init(const std::string &pakPath) {
    if (pak_.isOpen()) {
        return true;
    }
    if (!pak_.open(pakPath)) {
        std::fprintf(stderr, "[ResourceLoader] open pak failed: %s\n", pakPath.c_str());
        return false;
    }
    return true;
}

void ResourceLoader::shutdown() {
    // 先释放字体缓存（TTF_Font 依赖 SDL_ttf，尽量在 TTF_Quit 前调用）
    for (auto &kv : fontCache_) {
        if (kv.second.font) {
            TTF_CloseFont(kv.second.font);
            kv.second.font = nullptr;
        }
        if (kv.second.rw) {
            SDL_RWclose(kv.second.rw);
            kv.second.rw = nullptr;
        }
        kv.second.data.clear();
    }
    fontCache_.clear();

    if (pak_.isOpen()) {
        pak_.close();
    }
}

bool ResourceLoader::loadBytes(const std::string &name, std::vector<std::uint8_t> &outData) const {
    if (!pak_.isOpen()) {
        return false;
    }
    return pak_.readAll(name, outData);
}

SDL_Texture *ResourceLoader::loadTexture(SDL_Renderer *renderer, const std::string &name) const {
    if (!renderer) return nullptr;
    std::vector<std::uint8_t> data;
    if (!loadBytes(name, data) || data.empty()) {
        return nullptr;
    }

    SDL_RWops *rw = SDL_RWFromConstMem(data.data(), static_cast<int>(data.size()));
    if (!rw) {
        return nullptr;
    }

    SDL_Surface *surf = IMG_Load_RW(rw, 1); // 1 = 读取后自动关闭 rw
    if (!surf) {
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) {
        return nullptr;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

TTF_Font *ResourceLoader::loadFont(const std::string &name, int ptSize) {
    const std::string key = makeFontKey(name, ptSize);
    auto it = fontCache_.find(key);
    if (it != fontCache_.end()) {
        return it->second.font;
    }

    FontCacheEntry entry;
    if (!loadBytes(name, entry.data) || entry.data.empty()) {
        return nullptr;
    }

    entry.rw = SDL_RWFromConstMem(entry.data.data(), static_cast<int>(entry.data.size()));
    if (!entry.rw) {
        entry.data.clear();
        return nullptr;
    }

    // freesrc=0：TTF_Font 在使用期间可能会再次读取 RWops，因此由我们负责在 shutdown 时关闭 RWops
    entry.font = TTF_OpenFontRW(entry.rw, 0, ptSize);
    if (!entry.font) {
        SDL_RWclose(entry.rw);
        entry.rw = nullptr;
        entry.data.clear();
        return nullptr;
    }

    fontCache_[key] = entry;
    return fontCache_[key].font;
}

