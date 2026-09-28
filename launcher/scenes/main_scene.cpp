#include "main_scene.h"

#include "../launcher_theme.h"

void MainScene::onEnter() {
    menuInput_.resetStickState();
}

void MainScene::handleEvent(const SDL_Event &event, LauncherSceneInputContext &ctx) {
    static constexpr Uint8 BTN_B = 0;
    static constexpr Uint8 BTN_A = 1;

    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) {
            ctx.action.switchTo = LauncherSceneTarget::kSettings;
            return;
        }
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
            if (ctx.count > 0)
                ctx.action.confirm = true;
            return;
        }
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
            return;
        }
        menuInput_.handleControllerButton(event.cbutton.button, ctx.selected, ctx.count,
                                          ctx.action.confirm, ctx.action.quit);
    } else if (event.type == SDL_KEYDOWN) {
        menuInput_.handleKey(event.key.keysym.sym, ctx.selected, ctx.count, ctx.action.confirm,
                             ctx.action.quit, ctx.action.switchTo);
    } else if (event.type == SDL_CONTROLLERAXISMOTION) {
        menuInput_.handleControllerAxisMotion(event.caxis.axis, event.caxis.value, ctx.selected,
                                              ctx.count);
    } else if (event.type == SDL_JOYAXISMOTION) {
        Sint16 value = static_cast<Sint16>(event.jaxis.value);
        menuInput_.handleJoyAxisMotion(event.jaxis.axis, value, ctx.selected, ctx.count);
    } else if (event.type == SDL_JOYHATMOTION) {
        menuInput_.handleJoyHatMotion(event.jhat.value, ctx.selected, ctx.count);
    } else if (event.type == SDL_JOYBUTTONDOWN) {
        int btn = event.jbutton.button;
        if (btn == ctx.menuJoyButton) {
            ctx.action.switchTo = LauncherSceneTarget::kSettings;
        } else if (btn == BTN_B) {
            if (ctx.count > 0)
                ctx.action.confirm = true;
        }
    }
}

void MainScene::draw(const LauncherSceneRenderContext &ctx) {
    renderMainUi(ctx);
}

void MainScene::renderMainUi(const LauncherSceneRenderContext &ctx) {
    int windowW = LauncherTheme::kDesignWidth;
    int windowH = LauncherTheme::kDesignHeight;
    SDL_RenderGetLogicalSize(ctx.renderer, &windowW, &windowH);
    if (windowW <= 0 || windowH <= 0) {
        windowW = LauncherTheme::kDesignWidth;
        windowH = LauncherTheme::kDesignHeight;
    }

    chrome_.clearBackground(ctx.renderer, windowW, windowH);
    chrome_.drawBackgroundOverlay(ctx.textureCache, ctx.renderer, windowW, windowH);
    chrome_.drawTitle(ctx.textureCache, ctx.renderer, windowW);

    const int listTop = LauncherTheme::kListTopMargin;
    const int listBottomMargin = LauncherTheme::kListBottomMargin;
    int listHeight = windowH - listTop - listBottomMargin;
    if (listHeight < 0)
        listHeight = 0;
    SDL_Rect listClip = {0, listTop, windowW, listHeight};
    SDL_RenderSetClipRect(ctx.renderer, &listClip);

    if (ctx.games.empty()) {
        chrome_.drawEmptyListHint(ctx.font, ctx.renderer, windowW, windowH);
        SDL_RenderSetClipRect(ctx.renderer, nullptr);
        if (LauncherTheme::kLayout == LauncherTheme::Layout::kGrid)
            chrome_.drawBottomOpsHintImage(ctx.textureCache, ctx.renderer, windowW, windowH,
                                           LauncherTheme::kBottomOpsHintGridImage);
        else
            chrome_.drawBottomOpsHint(ctx.font, ctx.renderer, windowW, windowH,
                                       LauncherTheme::kBottomOpsHintDefault);
        return;
    }

    bool drewList = false;
    switch (LauncherTheme::kLayout) {
    case LauncherTheme::Layout::kGrid:
        drewList = gridView_.render(ctx.renderer, ctx.textureCache, ctx.games, ctx.selected,
                                    windowW, windowH, listClip);
        break;
    case LauncherTheme::Layout::kSwitchRow:
        drewList = switchRowView_.render(ctx.renderer, ctx.textureCache, ctx.games, ctx.selected,
                                         windowW, windowH, listClip);
        break;
    case LauncherTheme::Layout::kVertical:
    default:
        drewList = listView_.render(ctx.renderer, ctx.textureCache, ctx.games, ctx.selected,
                                    windowW, windowH, listClip);
        break;
    }
    SDL_RenderSetClipRect(ctx.renderer, nullptr);

    if (!drewList) {
        chrome_.drawBottomOpsHint(ctx.font, ctx.renderer, windowW, windowH,
                                  LauncherTheme::kBottomOpsHintEmptyRange);
        return;
    }

    if (LauncherTheme::kLayout == LauncherTheme::Layout::kGrid)
        chrome_.drawBottomOpsHintImage(ctx.textureCache, ctx.renderer, windowW, windowH,
                                      LauncherTheme::kBottomOpsHintGridImage);
    else
        chrome_.drawBottomOpsHint(ctx.font, ctx.renderer, windowW, windowH,
                                  LauncherTheme::kBottomOpsHintDefault);
}
