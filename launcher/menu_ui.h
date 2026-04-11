#ifndef LAUNCHER_MENU_UI_H
#define LAUNCHER_MENU_UI_H

#include "game_list.h"
#include "launcher_chrome.h"
#include "launcher_game_list_view_vertical.h"
#include "launcher_switch_game_row_view.h"
#include "launcher_menu_input.h"
#include "launcher_texture_cache.h"

#include <SDL.h>
#include <SDL_ttf.h>
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
    void openDevice(int index);
    void closeDevice();
    void handleDeviceAdded(int index);
    void handleDeviceRemoved(Sint32 instanceId);
    void render(const std::vector<GameEntry> &games, int selected);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    TTF_Font *font_ = nullptr;
    TTF_Font *titleFont_ = nullptr;
    SDL_GameController *controller_ = nullptr;
    SDL_Joystick *joystick_ = nullptr;
    std::string fontPath_;

    LauncherTextureCache textureCache_;
    VerticalGameListView listView_;
    SwitchGameRowView switchRowView_;
    LauncherChrome chrome_;
    LauncherMenuInput menuInput_;
};

#endif
