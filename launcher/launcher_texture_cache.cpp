#include "launcher_texture_cache.h"
#include "resource_loader.h"

#include <SDL_image.h>

void LauncherTextureCache::init(SDL_Renderer *renderer, TTF_Font *listFont) {
    renderer_ = renderer;
    listFont_ = listFont;
}

void LauncherTextureCache::clear() {
    for (auto &p : iconCache_) {
        if (p.second)
            SDL_DestroyTexture(p.second);
    }
    iconCache_.clear();
    for (auto &p : textCache_) {
        if (p.second.texture)
            SDL_DestroyTexture(p.second.texture);
    }
    textCache_.clear();
}

void LauncherTextureCache::resetTextureAlpha(SDL_Texture *tex, Uint8 alpha) {
    if (tex)
        SDL_SetTextureAlphaMod(tex, alpha);
}

SDL_Texture *LauncherTextureCache::getIconTexture(const std::string &iconPath) {
    if (!renderer_ || iconPath.empty())
        return nullptr;
    auto it = iconCache_.find(iconPath);
    if (it != iconCache_.end())
        return it->second;

    SDL_Texture *tex = nullptr;

    static const std::string kDataPrefix = "data/";
    if (iconPath.compare(0, kDataPrefix.size(), kDataPrefix) == 0 &&
        ResourceLoader::instance().isReady()) {
        std::string logicalName = iconPath.substr(kDataPrefix.size());
        tex = ResourceLoader::instance().loadTexture(renderer_, logicalName);
    }

    if (!tex) {
        SDL_Surface *surf = IMG_Load(iconPath.c_str());
        if (!surf)
            return nullptr;
        tex = SDL_CreateTextureFromSurface(renderer_, surf);
        SDL_FreeSurface(surf);
        if (!tex)
            return nullptr;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    }

    iconCache_[iconPath] = tex;
    return tex;
}

SDL_Texture *LauncherTextureCache::getTextTexture(const std::string &cacheKey,
                                                  const std::string &text,
                                                  const SDL_Color &color,
                                                  int &outW,
                                                  int &outH) {
    auto it = textCache_.find(cacheKey);
    if (it != textCache_.end()) {
        outW = it->second.width;
        outH = it->second.height;
        return it->second.texture;
    }

    if (!listFont_) {
        outW = outH = 0;
        return nullptr;
    }

    SDL_Surface *surf = TTF_RenderUTF8_Blended(listFont_, text.c_str(), color);
    if (!surf) {
        outW = outH = 0;
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        outW = outH = 0;
        return nullptr;
    }

    TextCacheEntry entry;
    entry.texture = tex;
    entry.width = surf->w;
    entry.height = surf->h;
    textCache_[cacheKey] = entry;

    outW = entry.width;
    outH = entry.height;

    SDL_FreeSurface(surf);
    return tex;
}
