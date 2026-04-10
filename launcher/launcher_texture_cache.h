#ifndef LAUNCHER_TEXTURE_CACHE_H
#define LAUNCHER_TEXTURE_CACHE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <unordered_map>

/**
 * 启动器图标与列表文字纹理缓存（原 MenuUI::getIconTexture / getTextTexture）。
 * 需在 ResourceLoader::init 之后使用 data/ 路径加载。
 */
class LauncherTextureCache {
public:
    LauncherTextureCache() = default;
    ~LauncherTextureCache() { clear(); }

    LauncherTextureCache(const LauncherTextureCache &) = delete;
    LauncherTextureCache &operator=(const LauncherTextureCache &) = delete;

    void init(SDL_Renderer *renderer, TTF_Font *listFont);
    void clear();

    SDL_Renderer *renderer() const { return renderer_; }
    TTF_Font *listFont() const { return listFont_; }

    SDL_Texture *getIconTexture(const std::string &iconPath);
    /** 绘制后若修改了纹理 alpha，可调用以恢复，避免污染缓存 */
    static void resetTextureAlpha(SDL_Texture *tex, Uint8 alpha = 255);

    SDL_Texture *getTextTexture(const std::string &cacheKey,
                                const std::string &text,
                                const SDL_Color &color,
                                int &outW,
                                int &outH);

private:
    struct TextCacheEntry {
        SDL_Texture *texture = nullptr;
        int width = 0;
        int height = 0;
    };

    SDL_Renderer *renderer_ = nullptr;
    TTF_Font *listFont_ = nullptr;
    std::unordered_map<std::string, SDL_Texture *> iconCache_;
    std::unordered_map<std::string, TextCacheEntry> textCache_;
};

#endif
