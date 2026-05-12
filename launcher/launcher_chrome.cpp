#include "launcher_chrome.h"
#include "launcher_texture_cache.h"

void LauncherChrome::clearBackground(SDL_Renderer *renderer, int windowW, int windowH) {
    (void)windowW;
    (void)windowH;
    SDL_Color c = LauncherTheme::bg();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(renderer);
}

void LauncherChrome::drawBackgroundOverlay(LauncherTextureCache &cache, SDL_Renderer *renderer,
                                           int windowW, int windowH) {
    SDL_Texture *bgTex = cache.getIconTexture("data/bg.jpg");
    if (bgTex) {
        SDL_SetTextureAlphaMod(bgTex, LauncherTheme::kBgOverlayAlpha);
        SDL_Rect dst = {0, 0, windowW, windowH};
        SDL_RenderCopy(renderer, bgTex, nullptr, &dst);
        LauncherTextureCache::resetTextureAlpha(bgTex, 255);
    }
}

void LauncherChrome::drawTitle(LauncherTextureCache &cache, SDL_Renderer *renderer, int windowW) {
    if (!renderer)
        return;
    SDL_Texture *tex = cache.getIconTexture(LauncherTheme::kTitleImage);
    if (!tex)
        return;
    int tw = 0;
    int th = 0;
    if (SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th) != 0 || tw <= 0 || th <= 0)
        return;
    int dstW = tw;
    int dstH = th;
    static constexpr int kTitleSideMargin = 16;
    if (dstW > windowW - kTitleSideMargin) {
        dstW = windowW - kTitleSideMargin;
        if (dstW < 1)
            dstW = 1;
        dstH = (th * dstW) / tw;
        if (dstH < 1)
            dstH = 1;
    }
    const int titleX = (windowW - dstW) / 2;
    int titleY = (LauncherTheme::kListTopMargin - dstH) / 2;
    if (titleY < 0)
        titleY = 0;
    const SDL_Rect dst = {titleX, titleY, dstW, dstH};
    SDL_RenderCopy(renderer, tex, nullptr, &dst);
}

void LauncherChrome::drawEmptyListHint(TTF_Font *listFont, SDL_Renderer *renderer, int windowW,
                                       int windowH) {
    if (!listFont)
        return;
    const char *hintText = LauncherTheme::kEmptyListHint;
    SDL_Surface *hintSurf = TTF_RenderUTF8_Blended(listFont, hintText, LauncherTheme::emptyHint());
    if (!hintSurf)
        return;
    SDL_Texture *hintTex = SDL_CreateTextureFromSurface(renderer, hintSurf);
    if (hintTex) {
        int textW = hintSurf->w;
        int textH = hintSurf->h;
        int textX = (windowW - textW) / 2;
        int textY = (windowH - textH) / 2;
        SDL_Rect dst = {textX, textY, textW, textH};
        SDL_RenderCopy(renderer, hintTex, nullptr, &dst);
        SDL_DestroyTexture(hintTex);
    }
    SDL_FreeSurface(hintSurf);
}

void LauncherChrome::drawBottomOpsHint(TTF_Font *listFont, SDL_Renderer *renderer, int windowW,
                                       int windowH, const char *hintOps, float scale) {
    if (!listFont || !hintOps)
        return;
    SDL_Surface *opsSurf = TTF_RenderUTF8_Blended(listFont, hintOps, LauncherTheme::hintOps());
    if (!opsSurf)
        return;
    SDL_Texture *opsTex = SDL_CreateTextureFromSurface(renderer, opsSurf);
    if (opsTex) {
        int textW = opsSurf->w;
        int textH = opsSurf->h;
        int dstW = static_cast<int>(textW * scale);
        int dstH = static_cast<int>(textH * scale);
        int padding = LauncherTheme::kBottomOpsPadding;
        int textX = windowW - dstW - padding;
        int textY = windowH - dstH;
        SDL_Rect dst = {textX, textY, dstW, dstH};
        SDL_RenderCopy(renderer, opsTex, nullptr, &dst);
        SDL_DestroyTexture(opsTex);
    }
    SDL_FreeSurface(opsSurf);
}

void LauncherChrome::drawBottomOpsHintImage(LauncherTextureCache &cache, SDL_Renderer *renderer,
                                             int windowW, int windowH, const char *iconPath,
                                             float scale) {
    if (!renderer || !iconPath)
        return;
    SDL_Texture *tex = cache.getIconTexture(iconPath);
    if (!tex)
        return;
    int tw = 0;
    int th = 0;
    if (SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th) != 0 || tw <= 0 || th <= 0)
        return;
    int dstW = static_cast<int>(tw * scale);
    int dstH = static_cast<int>(th * scale);
    if (dstW <= 0)
        dstW = tw;
    if (dstH <= 0)
        dstH = th;
    const int padding = LauncherTheme::kBottomOpsPadding;
    const int textX = windowW - dstW - padding;
    const int textY = windowH - dstH;
    const SDL_Rect dst = {textX, textY, dstW, dstH};
    SDL_RenderCopy(renderer, tex, nullptr, &dst);
}
