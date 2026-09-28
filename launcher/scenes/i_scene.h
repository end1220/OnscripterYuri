#ifndef LAUNCHER_SCENES_I_SCENE_H
#define LAUNCHER_SCENES_I_SCENE_H

#include "../game_list.h"
#include "../launcher_texture_cache.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

enum class LauncherSceneTarget {
    kNone,
    kMain,
    kSettings,
};

struct LauncherSceneAction {
    bool quit = false;
    bool confirm = false;
    LauncherSceneTarget switchTo = LauncherSceneTarget::kNone;
};

struct LauncherSceneInputContext {
    LauncherSceneInputContext(const std::vector<GameEntry> &gamesRef, int &selectedRef, int gameCount,
                              LauncherSceneAction &sceneAction)
        : games(gamesRef), selected(selectedRef), count(gameCount), action(sceneAction) {}

    const std::vector<GameEntry> &games;
    int &selected;
    int count;
    LauncherSceneAction &action;
    // 作为 MENU 的原始 Joystick 按键号；-1 表示设备映射了 GUIDE，不再认原始按键
    int menuJoyButton = -1;
};

struct LauncherSceneRenderContext {
    LauncherSceneRenderContext(SDL_Renderer *sdlRenderer, TTF_Font *listFont,
                               LauncherTextureCache &cache,
                               const std::vector<GameEntry> &gameEntries, int selectedIndex)
        : renderer(sdlRenderer), font(listFont), textureCache(cache), games(gameEntries),
          selected(selectedIndex) {}

    SDL_Renderer *renderer;
    TTF_Font *font;
    LauncherTextureCache &textureCache;
    const std::vector<GameEntry> &games;
    int selected;
};

class IScene {
public:
    virtual ~IScene() = default;

    virtual void onEnter() {}
    virtual void handleEvent(const SDL_Event &event, LauncherSceneInputContext &ctx) = 0;
    virtual void draw(const LauncherSceneRenderContext &ctx) = 0;
};

#endif
