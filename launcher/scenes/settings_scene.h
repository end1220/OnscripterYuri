#ifndef LAUNCHER_SCENES_SETTINGS_SCENE_H
#define LAUNCHER_SCENES_SETTINGS_SCENE_H

#include "i_scene.h"
#include "main_scene.h"

class SettingsScene : public IScene {
public:
    explicit SettingsScene(MainScene &mainScene);
    ~SettingsScene() override;

    void onEnter() override;
    void handleEvent(const SDL_Event &event, LauncherSceneInputContext &ctx) override;
    void draw(const LauncherSceneRenderContext &ctx) override;
    void captureBlurredBackground(const LauncherSceneRenderContext &ctx);

private:
    void focusNextButton();
    void focusPrevButton();
    void clearFocus();
    void confirmFocused(LauncherSceneAction &action);
    void drawPanel(const LauncherSceneRenderContext &ctx, const SDL_Rect &panel);
    void drawButtonText(const LauncherSceneRenderContext &ctx, const char *cachePrefix,
                        const char *text, bool selected, int centerX, int y);
    void drawAuthorProfile(const LauncherSceneRenderContext &ctx, const SDL_Rect &panel);
    void drawTextAt(const LauncherSceneRenderContext &ctx, const char *cacheKey, const char *text,
                    const SDL_Color &color, int leftX, int y, float scale, int shadowOffset);
    void drawText(const LauncherSceneRenderContext &ctx, const char *cacheKey, const char *text,
                  const SDL_Color &color, int centerX, int y, float scale, int shadowOffset);

    MainScene &mainScene_;
    int focusedButton_ = -1;
    SDL_Texture *blurredBackground_ = nullptr;
    int blurredW_ = 0;
    int blurredH_ = 0;
};

#endif
