#ifndef LAUNCHER_MENU_UI_H
#define LAUNCHER_MENU_UI_H

#include "game_list.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>
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
    struct TextCacheEntry {
        SDL_Texture *texture = nullptr;
        int width = 0;
        int height = 0;
    };

    void handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerButton(Uint8 button, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerAxis(Sint16 value, int &selected, int count);
    void handleJoyAxis(int axis, Sint16 value, int &selected, int count);
    void handleJoyHat(Uint8 value, int &selected, int count);
    bool trySelectUp(int &selected, int count);
    bool trySelectDown(int &selected, int count);
    void openDevice(int index);
    void closeDevice();
    void handleDeviceAdded(int index);
    void handleDeviceRemoved(Sint32 instanceId);
    void render(const std::vector<GameEntry> &games, int selected);
    SDL_Texture *getIconTexture(const std::string &iconPath);
    SDL_Texture *getTextTexture(const std::string &cacheKey,
                                const std::string &text,
                                const SDL_Color &color,
                                int &outW,
                                int &outH);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    TTF_Font *font_ = nullptr;       // 游戏列表文字字体
    TTF_Font *titleFont_ = nullptr;  // 顶部标题字体
    SDL_GameController *controller_ = nullptr;
    SDL_Joystick *joystick_ = nullptr;
    std::string fontPath_;
    int leftStickY_ = 0;
    int scrollOffset_ = 0;
    Uint32 lastSelectChangeTicks_ = 0;  // 防重复触发
    static const int AXIS_DEADZONE = 16000;
    static const Uint32 SELECT_DEBOUNCE_MS = 150;
    std::unordered_map<std::string, SDL_Texture *> iconCache_;  // iconPath -> 缓存的 150x150 纹理
    std::unordered_map<std::string, TextCacheEntry> textCache_; // 文本纹理缓存（按文字+颜色）
};

#endif
