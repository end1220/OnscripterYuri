#ifndef LAUNCHER_MENU_UI_H
#define LAUNCHER_MENU_UI_H

#include "game_list.h"
#include "launcher_texture_cache.h"
#include "scene_manager.h"
#include "scenes/main_scene.h"
#include "scenes/settings_scene.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <memory>
#include <string>
#include <vector>

class MenuUI {
public:
    MenuUI() = default;
    ~MenuUI() { shutdown(); }

    MenuUI(const MenuUI &) = delete;
    MenuUI &operator=(const MenuUI &) = delete;

    bool init(const std::string &fontPath = "", const std::string &launcherDataDir = "",
              bool windowed = false, int windowWidth = 960, int windowHeight = 720);
    void shutdown();

    int run(const std::vector<GameEntry> &games);

private:
    bool shouldIgnoreJoystickInputEvent(const SDL_Event &event) const;

    void openDevice(int index);
    void closeDevice();
    void handleDeviceAdded(int index);
    void handleDeviceRemoved(Sint32 instanceId);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    TTF_Font *font_ = nullptr;
    SDL_GameController *controller_ = nullptr;
    SDL_Joystick *joystick_ = nullptr;
    std::string fontPath_;

    LauncherTextureCache textureCache_;
    std::unique_ptr<MainScene> mainScene_;
    std::unique_ptr<SettingsScene> settingsScene_;
    std::unique_ptr<SceneManager> sceneManager_;
};

#endif
