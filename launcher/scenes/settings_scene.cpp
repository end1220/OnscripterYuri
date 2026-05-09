#include "settings_scene.h"

#include "../launcher_theme.h"

#include <string>
#include <vector>

namespace {
static void boxBlurPass(const std::vector<Uint32> &src, std::vector<Uint32> &dst, int w, int h,
                        int radius, bool horizontal) {
    if (radius <= 0) {
        dst = src;
        return;
    }
    const int size = radius * 2 + 1;
    dst.resize(src.size());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int a = 0, r = 0, g = 0, b = 0;
            for (int i = -radius; i <= radius; ++i) {
                int sx = x;
                int sy = y;
                if (horizontal)
                    sx = x + i;
                else
                    sy = y + i;
                if (sx < 0)
                    sx = 0;
                if (sx >= w)
                    sx = w - 1;
                if (sy < 0)
                    sy = 0;
                if (sy >= h)
                    sy = h - 1;
                Uint32 p = src[sy * w + sx];
                a += (p >> 24) & 0xFF;
                b += (p >> 16) & 0xFF;
                g += (p >> 8) & 0xFF;
                r += p & 0xFF;
            }
            Uint32 outA = static_cast<Uint32>(a / size);
            Uint32 outB = static_cast<Uint32>(b / size);
            Uint32 outG = static_cast<Uint32>(g / size);
            Uint32 outR = static_cast<Uint32>(r / size);
            dst[y * w + x] = (outA << 24) | (outB << 16) | (outG << 8) | outR;
        }
    }
}
} // namespace

SettingsScene::SettingsScene(MainScene &mainScene) : mainScene_(mainScene) {}

SettingsScene::~SettingsScene() {
    if (blurredBackground_) {
        SDL_DestroyTexture(blurredBackground_);
        blurredBackground_ = nullptr;
    }
}

void SettingsScene::onEnter() {
    focusedButton_ = -1;
    axisXState_ = 0;
    axisYState_ = 0;
    lastFocusMoveTicks_ = 0;
}

bool SettingsScene::canMoveFocus() {
    static const Uint32 kDebounceMs = 130;
    Uint32 now = SDL_GetTicks();
    if (now - lastFocusMoveTicks_ < kDebounceMs)
        return false;
    lastFocusMoveTicks_ = now;
    return true;
}

void SettingsScene::focusNextButton() {
    static const int kButtonCount = 4;
    if (!canMoveFocus())
        return;
    if (focusedButton_ < 0)
        focusedButton_ = 0;
    else
        focusedButton_ = (focusedButton_ + 1) % kButtonCount;
}

void SettingsScene::focusPrevButton() {
    static const int kButtonCount = 4;
    if (!canMoveFocus())
        return;
    if (focusedButton_ < 0)
        focusedButton_ = kButtonCount - 1;
    else
        focusedButton_ = (focusedButton_ + kButtonCount - 1) % kButtonCount;
}

void SettingsScene::clearFocus() {
    focusedButton_ = -1;
}

void SettingsScene::confirmFocused(LauncherSceneAction &action) {
    if (focusedButton_ == 0) {
        action.switchTo = LauncherSceneTarget::kMain;
    } else if (focusedButton_ == 3) {
        action.quit = true;
    }
}

void SettingsScene::handleEvent(const SDL_Event &event, LauncherSceneInputContext &ctx) {
    static constexpr Uint8 BTN_B = 0;
    static constexpr Uint8 BTN_A = 1;
    static constexpr Uint8 BTN_MENU = 10;

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_m:
            ctx.action.switchTo = LauncherSceneTarget::kMain;
            break;
        case SDLK_UP:
        case SDLK_LEFT:
            focusPrevButton();
            break;
        case SDLK_DOWN:
        case SDLK_RIGHT:
        case SDLK_TAB:
            focusNextButton();
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            confirmFocused(ctx.action);
            break;
        case SDLK_b:
            if (focusedButton_ >= 0)
                clearFocus();
            else
                ctx.action.switchTo = LauncherSceneTarget::kMain;
            break;
        case SDLK_ESCAPE:
            ctx.action.switchTo = LauncherSceneTarget::kMain;
            break;
        default:
            break;
        }
    } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        if (event.cbutton.button == BTN_MENU) {
            ctx.action.switchTo = LauncherSceneTarget::kMain;
            return;
        }
        switch (event.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            focusPrevButton();
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            focusNextButton();
            break;
        case SDL_CONTROLLER_BUTTON_B:
            confirmFocused(ctx.action);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            if (focusedButton_ >= 0)
                clearFocus();
            else
                ctx.action.switchTo = LauncherSceneTarget::kMain;
            break;
        default:
            break;
        }
    } else if (event.type == SDL_JOYHATMOTION) {
        if (event.jhat.value & (SDL_HAT_UP | SDL_HAT_LEFT))
            focusPrevButton();
        else if (event.jhat.value & (SDL_HAT_DOWN | SDL_HAT_RIGHT))
            focusNextButton();
    } else if (event.type == SDL_CONTROLLERAXISMOTION) {
        static const Sint16 kDeadzone = 16000;
        if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
            if (event.caxis.value < -kDeadzone) {
                if (axisYState_ != -1) {
                    focusPrevButton();
                    axisYState_ = -1;
                }
            } else if (event.caxis.value > kDeadzone) {
                if (axisYState_ != 1) {
                    focusNextButton();
                    axisYState_ = 1;
                }
            } else {
                axisYState_ = 0;
            }
        }
    } else if (event.type == SDL_JOYAXISMOTION) {
        static const Sint16 kDeadzone = 16000;
        if (event.jaxis.axis == 1) {
            Sint16 value = static_cast<Sint16>(event.jaxis.value);
            if (value < -kDeadzone) {
                if (axisYState_ != -1) {
                    focusPrevButton();
                    axisYState_ = -1;
                }
            } else if (value > kDeadzone) {
                if (axisYState_ != 1) {
                    focusNextButton();
                    axisYState_ = 1;
                }
            } else {
                axisYState_ = 0;
            }
        }
    } else if (event.type == SDL_JOYBUTTONDOWN) {
        int btn = event.jbutton.button;
        if (btn == BTN_MENU) {
            ctx.action.switchTo = LauncherSceneTarget::kMain;
        } else if (btn == BTN_B) {
            confirmFocused(ctx.action);
        } else if (btn == BTN_A) {
            if (focusedButton_ >= 0)
                clearFocus();
            else
                ctx.action.switchTo = LauncherSceneTarget::kMain;
        }
    }
}

void SettingsScene::draw(const LauncherSceneRenderContext &ctx) {
    int windowW = LauncherTheme::kDesignWidth;
    int windowH = LauncherTheme::kDesignHeight;
    SDL_RenderGetLogicalSize(ctx.renderer, &windowW, &windowH);
    if (windowW <= 0 || windowH <= 0) {
        windowW = LauncherTheme::kDesignWidth;
        windowH = LauncherTheme::kDesignHeight;
    }

    if (blurredBackground_) {
        SDL_Rect full = {0, 0, windowW, windowH};
        SDL_RenderCopy(ctx.renderer, blurredBackground_, nullptr, &full);
    } else {
        mainScene_.draw(ctx);
    }

    SDL_Color backdropColor = LauncherTheme::settingsBackdrop();
    SDL_SetRenderDrawColor(ctx.renderer, backdropColor.r, backdropColor.g, backdropColor.b,
                           backdropColor.a);
    SDL_Rect fullScreen = {0, 0, windowW, windowH};
    SDL_RenderFillRect(ctx.renderer, &fullScreen);

    int panelSize =
        static_cast<int>((windowW < windowH ? windowW : windowH) * LauncherTheme::kSettingsPanelSizeRatio);
    if (panelSize <= 0)
        panelSize = LauncherTheme::kSettingsPanelSize;
    if (panelSize > windowW)
        panelSize = windowW;
    if (panelSize > windowH)
        panelSize = windowH;
    int panelW = static_cast<int>(panelSize * 1.17f);
    if (panelW > windowW)
        panelW = windowW;
    SDL_Rect panel = {(windowW - panelW) / 2, (windowH - panelSize) / 2, panelW, panelSize};

    drawPanel(ctx, panel);

    const int centerX = panel.x + panel.h / 5;
    const int firstButtonY = panel.y + panel.h / 6;
    const int secondButtonY = firstButtonY + 50;
    const int thirdButtonY = secondButtonY + 50;
    const int fourthButtonY = thirdButtonY + 50;
    drawButtonText(ctx, "settings_return", LauncherTheme::kSettingsReturnText, focusedButton_ == 0,
                   centerX, firstButtonY);
    drawButtonText(ctx, "settings_author", LauncherTheme::kSettingsAuthorTabText, focusedButton_ == 1,
                   centerX, secondButtonY);
    drawButtonText(ctx, "settings_about", LauncherTheme::kSettingsAboutTabText, focusedButton_ == 2,
                   centerX, thirdButtonY);
    drawButtonText(ctx, "settings_exit", LauncherTheme::kSettingsExitText, focusedButton_ == 3,
                   centerX, fourthButtonY);

    if (focusedButton_ == 1) {
        drawAuthorProfile(ctx, panel);
    } else if (focusedButton_ == 2) {
        int areaLeft = panel.x + panel.w / 2 - 116;
        int areaTop = panel.y + panel.h / 6;
        drawTextAt(ctx, "settings_about_line1", LauncherTheme::kSettingsAboutLine1Text,
                   LauncherTheme::settingsText(), areaLeft, areaTop, 0.95f, 1);
        drawTextAt(ctx, "settings_about_line2", LauncherTheme::kSettingsAboutLine2Text,
                   LauncherTheme::settingsText(), areaLeft, areaTop + 36, 0.85f, 0);
        drawTextAt(ctx, "settings_about_line3", LauncherTheme::kSettingsAboutLine3Text,
                   LauncherTheme::settingsText(), areaLeft, areaTop + 68, 0.85f, 0);
        drawTextAt(ctx, "settings_about_line4", LauncherTheme::kSettingsAboutLine4Text,
                   LauncherTheme::settingsText(), areaLeft, areaTop + 100, 0.85f, 0);
    }

    drawText(ctx, "settings_signature", LauncherTheme::kSettingsSignatureText,
             LauncherTheme::gameNameSelected(), panel.x + panel.w / 2,
             panel.y + 14, 1.0f, 0);
}

void SettingsScene::captureBlurredBackground(const LauncherSceneRenderContext &ctx) {
    int windowW = LauncherTheme::kDesignWidth;
    int windowH = LauncherTheme::kDesignHeight;
    SDL_RenderGetLogicalSize(ctx.renderer, &windowW, &windowH);
    if (windowW <= 0 || windowH <= 0)
        return;

    SDL_Texture *captureTarget =
        SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windowW,
                          windowH);
    if (!captureTarget)
        return;

    SDL_Texture *oldTarget = SDL_GetRenderTarget(ctx.renderer);
    if (SDL_SetRenderTarget(ctx.renderer, captureTarget) != 0) {
        SDL_DestroyTexture(captureTarget);
        return;
    }

    mainScene_.draw(ctx);

    std::vector<Uint32> pixels(static_cast<size_t>(windowW) * static_cast<size_t>(windowH));
    if (SDL_RenderReadPixels(ctx.renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, pixels.data(),
                             windowW * static_cast<int>(sizeof(Uint32))) != 0) {
        SDL_SetRenderTarget(ctx.renderer, oldTarget);
        SDL_DestroyTexture(captureTarget);
        return;
    }
    SDL_SetRenderTarget(ctx.renderer, oldTarget);
    SDL_DestroyTexture(captureTarget);

    std::vector<Uint32> temp;
    std::vector<Uint32> blurA;
    std::vector<Uint32> blurB;
    // 双次盒模糊近似高斯，开销可控且仅在进入设置页时做一次。
    boxBlurPass(pixels, temp, windowW, windowH, 3, true);
    boxBlurPass(temp, blurA, windowW, windowH, 3, false);
    boxBlurPass(blurA, temp, windowW, windowH, 3, true);
    boxBlurPass(temp, blurB, windowW, windowH, 3, false);

    SDL_Texture *newTex =
        SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, windowW,
                          windowH);
    if (!newTex)
        return;
    SDL_SetTextureBlendMode(newTex, SDL_BLENDMODE_BLEND);
    if (SDL_UpdateTexture(newTex, nullptr, blurB.data(), windowW * static_cast<int>(sizeof(Uint32))) !=
        0) {
        SDL_DestroyTexture(newTex);
        return;
    }

    if (blurredBackground_)
        SDL_DestroyTexture(blurredBackground_);
    blurredBackground_ = newTex;
    blurredW_ = windowW;
    blurredH_ = windowH;
}

void SettingsScene::drawPanel(const LauncherSceneRenderContext &ctx, const SDL_Rect &panel) {
    SDL_Color panelColor = LauncherTheme::settingsPanel();
    SDL_Texture *panelTex = ctx.textureCache.getIconTexture("data/panel.png");
    if (panelTex) {
        SDL_SetTextureColorMod(panelTex, panelColor.r, panelColor.g, panelColor.b);
        SDL_SetTextureAlphaMod(panelTex, panelColor.a);
        SDL_RenderCopy(ctx.renderer, panelTex, nullptr, &panel);
        SDL_SetTextureColorMod(panelTex, 255, 255, 255);
        LauncherTextureCache::resetTextureAlpha(panelTex, 255);
    } else {
        SDL_SetRenderDrawColor(ctx.renderer, panelColor.r, panelColor.g, panelColor.b, panelColor.a);
        SDL_RenderFillRect(ctx.renderer, &panel);
    }
}

void SettingsScene::drawButtonText(const LauncherSceneRenderContext &ctx, const char *cachePrefix,
                                   const char *text, bool selected, int centerX, int y) {
    const float scale = selected ? 1.2f : 1.1f;
    const SDL_Color color = selected ? LauncherTheme::gameNameSelected() : LauncherTheme::settingsText();
    const char *state = selected ? "selected" : "normal";
    std::string textKey = std::string(cachePrefix) + "_" + state;
    int textW = 0;
    int textH = 0;
    SDL_Texture *textTex = ctx.textureCache.getTextTexture(textKey, text, color, textW, textH);
    if (!textTex)
        return;

    int drawTextW = static_cast<int>(textW * scale);
    if (drawTextW <= 0)
        drawTextW = textW;
    int textLeftX = centerX - drawTextW / 2;

    if (selected) {
        int arrowW = 0;
        int arrowH = 0;
        std::string arrowKey = std::string(cachePrefix) + "_arrow";
        SDL_Texture *arrowTex =
            ctx.textureCache.getTextTexture(arrowKey.c_str(), ">  ", color, arrowW, arrowH);
        int drawArrowW = static_cast<int>(arrowW * scale);
        if (drawArrowW <= 0)
            drawArrowW = arrowW;
        drawTextAt(ctx, arrowKey.c_str(), ">  ", color, textLeftX - drawArrowW, y, scale, 2);
    }

    drawTextAt(ctx, textKey.c_str(), text, color, textLeftX, y, scale, 2);
}

void SettingsScene::drawAuthorProfile(const LauncherSceneRenderContext &ctx, const SDL_Rect &panel) {
    int areaLeft = panel.x + panel.w / 2 - 96;
    int areaTop = panel.y + panel.h / 6;
    int imageSize = panel.h / 4;
    imageSize = static_cast<int>(imageSize * 1.3f);
    if (imageSize > 160)
        imageSize = 160;
    if (imageSize < 72)
        imageSize = 72;

    SDL_Texture *profile = ctx.textureCache.getIconTexture("data/profile.jpg");
    if (profile) {
        SDL_Rect imgDst = {areaLeft, areaTop, imageSize, imageSize};
        SDL_RenderCopy(ctx.renderer, profile, nullptr, &imgDst);
    }

    int textStartY = areaTop + imageSize + 20;
    drawTextAt(ctx, "settings_author_name", LauncherTheme::kSettingsAuthorNameText,
               LauncherTheme::settingsText(), areaLeft, textStartY, 1.0f, 1);
    drawTextAt(ctx, "settings_author_link", LauncherTheme::kSettingsAuthorLinkText,
               LauncherTheme::settingsText(), areaLeft, textStartY + 34, 0.75f, 0);
}

void SettingsScene::drawTextAt(const LauncherSceneRenderContext &ctx, const char *cacheKey,
                               const char *text, const SDL_Color &color, int leftX, int y,
                               float scale, int shadowOffset) {
    int textW = 0;
    int textH = 0;
    SDL_Texture *tex = ctx.textureCache.getTextTexture(cacheKey, text, color, textW, textH);
    if (!tex)
        return;

    int drawW = static_cast<int>(textW * scale);
    int drawH = static_cast<int>(textH * scale);
    if (drawW <= 0)
        drawW = textW;
    if (drawH <= 0)
        drawH = textH;

    if (shadowOffset > 0) {
        int sw = 0;
        int sh = 0;
        std::string shadowKey = std::string(cacheKey) + "_shadow";
        SDL_Color shadowColor = {0, 0, 0, 255};
        SDL_Texture *shadow = ctx.textureCache.getTextTexture(shadowKey, text, shadowColor, sw, sh);
        if (shadow) {
            int shadowW = static_cast<int>(sw * scale);
            int shadowH = static_cast<int>(sh * scale);
            if (shadowW <= 0)
                shadowW = sw;
            if (shadowH <= 0)
                shadowH = sh;
            SDL_Rect shadowDst = {leftX + shadowOffset, y + shadowOffset, shadowW, shadowH};
            SDL_RenderCopy(ctx.renderer, shadow, nullptr, &shadowDst);
        }
    }

    SDL_Rect dst = {leftX, y, drawW, drawH};
    SDL_RenderCopy(ctx.renderer, tex, nullptr, &dst);
}

void SettingsScene::drawText(const LauncherSceneRenderContext &ctx, const char *cacheKey,
                             const char *text, const SDL_Color &color, int centerX, int y,
                             float scale, int shadowOffset) {
    int textW = 0;
    int textH = 0;
    SDL_Texture *tex = ctx.textureCache.getTextTexture(cacheKey, text, color, textW, textH);
    if (!tex)
        return;

    int drawW = static_cast<int>(textW * scale);
    int drawH = static_cast<int>(textH * scale);
    if (drawW <= 0)
        drawW = textW;
    if (drawH <= 0)
        drawH = textH;
    SDL_Rect dst = {centerX - drawW / 2, y, drawW, drawH};

    drawTextAt(ctx, cacheKey, text, color, dst.x, y, scale, shadowOffset);
}
