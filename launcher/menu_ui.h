#ifndef LAUNCHER_MENU_UI_H
#define LAUNCHER_MENU_UI_H

#include "game_list.h"

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

    bool init(const std::string &fontPath = "");
    void shutdown();

    int run(const std::vector<GameEntry> &games);

private:
    void handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerButton(Uint8 button, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerAxis(Sint16 value, int &selected, int count);
    void render(const std::vector<GameEntry> &games, int selected);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    TTF_Font *font_ = nullptr;
    std::string fontPath_;
    int leftStickY_ = 0;
    static const int AXIS_DEADZONE = 16000;
};

#endif
