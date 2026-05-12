#include "launcher_texture_cache.h"
#include "resource_loader.h"

#include <SDL_image.h>

#include <string>

namespace {
static std::string makeTextCacheKey(const std::string &prefix, const std::string &cacheKey,
                                    const std::string &text, const SDL_Color &color,
                                    int wrapWidth) {
    return prefix + cacheKey + "|" + std::to_string(wrapWidth) + "|" +
           std::to_string(color.r) + "," + std::to_string(color.g) + "," +
           std::to_string(color.b) + "," + std::to_string(color.a) + "|" + text;
}
} // namespace

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
    for (auto &p : maskedIconCache_) {
        if (p.second)
            SDL_DestroyTexture(p.second);
    }
    maskedIconCache_.clear();
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

SDL_Texture *LauncherTextureCache::getMaskedIconTexture(const std::string &iconPath,
                                                        const std::string &maskPath,
                                                        int size) {
    if (!renderer_ || iconPath.empty() || maskPath.empty() || size <= 0)
        return getIconTexture(iconPath);

    const std::string cacheKey =
        std::string("masked:") + iconPath + "|" + maskPath + "|" + std::to_string(size);
    auto it = maskedIconCache_.find(cacheKey);
    if (it != maskedIconCache_.end())
        return it->second;

    SDL_Texture *iconTex = getIconTexture(iconPath);
    SDL_Texture *maskTex = getIconTexture(maskPath);
    if (!iconTex || !maskTex)
        return iconTex;

    SDL_Texture *targetTex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET, size, size);
    if (!targetTex)
        return iconTex;
    SDL_SetTextureBlendMode(targetTex, SDL_BLENDMODE_BLEND);

    SDL_Texture *prevTarget = SDL_GetRenderTarget(renderer_);
    if (SDL_SetRenderTarget(renderer_, targetTex) != 0) {
        SDL_DestroyTexture(targetTex);
        return iconTex;
    }

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_RenderClear(renderer_);

    SDL_Rect fullDst = {0, 0, size, size};
    SDL_RenderCopy(renderer_, iconTex, nullptr, &fullDst);

    SDL_BlendMode oldMaskBlend = SDL_BLENDMODE_BLEND;
    SDL_GetTextureBlendMode(maskTex, &oldMaskBlend);
    SDL_BlendMode maskBlend = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
    SDL_SetTextureBlendMode(maskTex, maskBlend);
    SDL_RenderCopy(renderer_, maskTex, nullptr, &fullDst);
    SDL_SetTextureBlendMode(maskTex, oldMaskBlend);

    SDL_SetRenderTarget(renderer_, prevTarget);

    maskedIconCache_[cacheKey] = targetTex;
    return targetTex;
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

SDL_Texture *LauncherTextureCache::getWrappedTextTexture(const std::string &cacheKey,
                                                         const std::string &text,
                                                         const SDL_Color &color,
                                                         int wrapWidth,
                                                         int &outW,
                                                         int &outH) {
    if (wrapWidth <= 0)
        return getTextTexture(cacheKey, text, color, outW, outH);

    const std::string wrappedKey = makeTextCacheKey("wrapped:", cacheKey, text, color, wrapWidth);
    auto it = textCache_.find(wrappedKey);
    if (it != textCache_.end()) {
        outW = it->second.width;
        outH = it->second.height;
        return it->second.texture;
    }

    if (!listFont_) {
        outW = outH = 0;
        return nullptr;
    }

    SDL_Surface *surf =
        TTF_RenderUTF8_Blended_Wrapped(listFont_, text.c_str(), color,
                                       static_cast<Uint32>(wrapWidth));
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
    textCache_[wrappedKey] = entry;

    outW = entry.width;
    outH = entry.height;

    SDL_FreeSurface(surf);
    return tex;
}
