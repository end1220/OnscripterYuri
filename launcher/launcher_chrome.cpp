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

void LauncherChrome::drawTitle(TTF_Font *titleFont, SDL_Renderer *renderer, int windowW) {
    if (!titleFont)
        return;
    const char *titleText = LauncherTheme::kTitleText;
    const int kTitleOutlinePx = LauncherTheme::kTitleOutlinePx;

    int titleX = 0;
    int titleY = 0;
    int titleW = 0;
    int titleH = 0;

    TTF_SetFontOutline(titleFont, kTitleOutlinePx);
    SDL_Surface *strokeSurf =
        TTF_RenderUTF8_Blended(titleFont, titleText, LauncherTheme::titleStroke());
    if (strokeSurf) {
        SDL_Texture *strokeTex = SDL_CreateTextureFromSurface(renderer, strokeSurf);
        if (strokeTex) {
            titleW = strokeSurf->w;
            titleH = strokeSurf->h;
            titleX = (windowW - titleW) / 2;
            titleY = (LauncherTheme::kListTopMargin - titleH) / 2;
            SDL_Rect dst = {titleX, titleY, titleW, titleH};
            SDL_RenderCopy(renderer, strokeTex, nullptr, &dst);
            SDL_DestroyTexture(strokeTex);
        }
        SDL_FreeSurface(strokeSurf);
    }

    TTF_SetFontOutline(titleFont, 0);
    SDL_Surface *fillSurf = TTF_RenderUTF8_Blended(titleFont, titleText, LauncherTheme::titleFill());
    if (fillSurf) {
        SDL_Texture *fillTex = SDL_CreateTextureFromSurface(renderer, fillSurf);
        if (fillTex) {
            if (titleW == 0 || titleH == 0) {
                titleW = fillSurf->w;
                titleH = fillSurf->h;
                titleX = (windowW - titleW) / 2;
                titleY = (LauncherTheme::kListTopMargin - titleH) / 2;
            }
            SDL_Rect dst = {titleX, titleY, fillSurf->w, fillSurf->h};
            SDL_RenderCopy(renderer, fillTex, nullptr, &dst);
            SDL_DestroyTexture(fillTex);
        }
        SDL_FreeSurface(fillSurf);
    }
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
