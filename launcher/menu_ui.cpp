#include "menu_ui.h"

#include <SDL_image.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

bool MenuUI::init(const std::string &fontPath) {
    fontPath_ = fontPath;

    Uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;

    // 尝试使用默认视频驱动初始化
    int numDrivers = SDL_GetNumVideoDrivers();
    bool inited = false;
    if (SDL_Init(sdlFlags) == 0) {
        inited = true;
        std::fprintf(stderr, "[Launcher] Using default video driver: %s\n",
                     SDL_GetCurrentVideoDriver());
    } else {
        std::fprintf(stderr, "[Launcher] SDL_Init default failed: %s\n", SDL_GetError());
        // 依次尝试每个可用的视频驱动
        for (int i = 0; i < numDrivers; ++i) {
            const char *drv = SDL_GetVideoDriver(i);
            if (!drv) continue;
            std::fprintf(stderr, "[Launcher] Try video driver: %s\n", drv);
            SDL_Quit();
            setenv("SDL_VIDEODRIVER", drv, 1);
            if (SDL_Init(sdlFlags) == 0) {
                inited = true;
                std::fprintf(stderr, "[Launcher] Using video driver: %s\n",
                             SDL_GetCurrentVideoDriver());
                break;
            }
            std::fprintf(stderr, "[Launcher] Driver \"%s\" failed: %s\n",
                         drv, SDL_GetError());
        }
    }

    if (!inited) {
        std::fprintf(stderr, "SDL init failed: no working video driver. %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::fprintf(stderr, "IMG_Init PNG failed: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // 全屏窗口，适配掌机
    window_ = SDL_CreateWindow(
        "ONS Launcher",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        0, 0,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    const char *path = fontPath_.empty() ? "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
                                          : fontPath_.c_str();
    SDL_JoystickEventState(SDL_ENABLE);
    if (SDL_NumJoysticks() > 0)
        openDevice(0);

    font_ = TTF_OpenFont(path, 24);
    if (!font_) {
        std::fprintf(stderr, "TTF_OpenFont failed for %s: %s\n", path, TTF_GetError());
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void MenuUI::openDevice(int index) {
    closeDevice();
    if (SDL_IsGameController(index)) {
        controller_ = SDL_GameControllerOpen(index);
        if (controller_)
            std::fprintf(stderr, "[Launcher] Controller: %s\n",
                         SDL_GameControllerName(controller_));
    } else {
        joystick_ = SDL_JoystickOpen(index);
        if (joystick_)
            std::fprintf(stderr, "[Launcher] Joystick: %s\n",
                         SDL_JoystickName(joystick_));
    }
}

void MenuUI::closeDevice() {
    if (controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
    if (joystick_) {
        SDL_JoystickClose(joystick_);
        joystick_ = nullptr;
    }
}

void MenuUI::handleDeviceAdded(int index) {
    if (!controller_ && !joystick_)
        openDevice(index);
}

void MenuUI::handleDeviceRemoved(Sint32 instanceId) {
    Sint32 ourId = -1;
    if (controller_) {
        SDL_Joystick *j = SDL_GameControllerGetJoystick(controller_);
        if (j) ourId = SDL_JoystickInstanceID(j);
    } else if (joystick_) {
        ourId = SDL_JoystickInstanceID(joystick_);
    }
    if (ourId >= 0 && ourId == instanceId) {
        std::fprintf(stderr, "[Launcher] Joystick/Controller removed.\n");
        closeDevice();
    }
}

void MenuUI::shutdown() {
    closeDevice();
    for (auto &p : iconCache_) {
        if (p.second) SDL_DestroyTexture(p.second);
    }
    iconCache_.clear();
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void MenuUI::handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm, bool &quit) {
    switch (sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
        quit = true;
        break;
    case SDLK_UP:
        trySelectUp(selected, count);
        break;
    case SDLK_DOWN:
        trySelectDown(selected, count);
        break;
    case SDLK_RETURN:
    case SDLK_SPACE:
        confirm = true;
        break;
    default:
        break;
    }
}

void MenuUI::handleControllerButton(Uint8 button, int &selected, int count, bool &confirm, bool &quit) {
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:
    case SDL_CONTROLLER_BUTTON_B:
        confirm = true;
        std::fprintf(stderr, "[Launcher] CONFIRM by controller button=%d selected=%d/%d\n",
                     static_cast<int>(button), selected, count);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        trySelectUp(selected, count);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        trySelectDown(selected, count);
        break;
    default:
        break;
    }
}

SDL_Texture *MenuUI::getIconTexture(const std::string &iconPath) {
    if (iconPath.empty()) return nullptr;
    auto it = iconCache_.find(iconPath);
    if (it != iconCache_.end()) return it->second;
    SDL_Surface *surf = IMG_Load(iconPath.c_str());
    if (!surf) return nullptr;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
    SDL_FreeSurface(surf);
    if (!tex) return nullptr;
    iconCache_[iconPath] = tex;
    return tex;
}

bool MenuUI::trySelectUp(int &selected, int count) {
    Uint32 now = SDL_GetTicks();
    if (now - lastSelectChangeTicks_ < SELECT_DEBOUNCE_MS)
        return false;
    if (selected > 0) --selected;
    else selected = count - 1;
    lastSelectChangeTicks_ = now;
    return true;
}

bool MenuUI::trySelectDown(int &selected, int count) {
    Uint32 now = SDL_GetTicks();
    if (now - lastSelectChangeTicks_ < SELECT_DEBOUNCE_MS)
        return false;
    if (selected < count - 1) ++selected;
    else selected = 0;
    lastSelectChangeTicks_ = now;
    return true;
}

void MenuUI::handleJoyAxis(int axis, Sint16 value, int &selected, int count) {
    if (axis != 1) return;  // axis 1 = Y
    handleControllerAxis(value, selected, count);
}

void MenuUI::handleJoyHat(Uint8 value, int &selected, int count) {
    if (value & SDL_HAT_UP)
        trySelectUp(selected, count);
    else if (value & SDL_HAT_DOWN)
        trySelectDown(selected, count);
}

void MenuUI::handleControllerAxis(Sint16 value, int &selected, int count) {
    if (value < -AXIS_DEADZONE) {
        if (leftStickY_ >= -AXIS_DEADZONE)
            trySelectUp(selected, count);
        leftStickY_ = value;
    } else if (value > AXIS_DEADZONE) {
        if (leftStickY_ <= AXIS_DEADZONE)
            trySelectDown(selected, count);
        leftStickY_ = value;
    } else {
        leftStickY_ = 0;
    }
}

// 布局常量
static const int ICON_SIZE = 150;
static const int ICON_TEXT_GAP = 16;
static const int ROW_HEIGHT = 170;   // 每行绘制高度
static const int ROW_GAP = 5;        // 两个条目之间的间距
static const int SLOT_HEIGHT = ROW_HEIGHT + ROW_GAP;

// Galgame 粉红桃色配色（可在此修改）
static const SDL_Color COLOR_BG = {45, 25, 45, 255};           // 背景
static const SDL_Color COLOR_TEXT = {255, 220, 230, 255};      // 未选中文字
static const SDL_Color COLOR_TEXT_SEL = {255, 182, 193, 255};  // 选中文字
static const SDL_Color COLOR_ROW_SEL = {90, 50, 75, 255};      // 选中行背景
static const SDL_Color COLOR_ICON_PLACEHOLDER = {255, 255, 255, 255};  // 无图标占位（纯白）

void MenuUI::render(const std::vector<GameEntry> &games, int selected) {
    SDL_SetRenderDrawColor(renderer_, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, COLOR_BG.a);
    SDL_RenderClear(renderer_);

    int windowW = 0, windowH = 0;
    SDL_GetRendererOutputSize(renderer_, &windowW, &windowH);

    int totalHeight = 20 + static_cast<int>(games.size()) * SLOT_HEIGHT - ROW_GAP + 20;
    int maxScroll = (totalHeight > windowH) ? (totalHeight - windowH) : 0;
    int itemTop = 20 + selected * SLOT_HEIGHT;
    int itemBottom = 20 + selected * SLOT_HEIGHT + ROW_HEIGHT;
    if (itemTop - scrollOffset_ < 20)
        scrollOffset_ = itemTop - 20;
    else if (itemBottom - scrollOffset_ > windowH - 20)
        scrollOffset_ = itemBottom - (windowH - 20);
    if (scrollOffset_ < 0) scrollOffset_ = 0;
    if (scrollOffset_ > maxScroll) scrollOffset_ = maxScroll;

    const int baseY = 20;
    for (std::size_t i = 0; i < games.size(); ++i) {
        int y = baseY + static_cast<int>(i) * SLOT_HEIGHT - scrollOffset_;
        SDL_Color color = COLOR_TEXT;
        if (static_cast<int>(i) == selected) color = COLOR_TEXT_SEL;

        int rowH = ROW_HEIGHT;
        if (y + rowH < 0 || y > windowH)
            continue;

        if (static_cast<int>(i) == selected) {
            SDL_SetRenderDrawColor(renderer_, COLOR_ROW_SEL.r, COLOR_ROW_SEL.g, COLOR_ROW_SEL.b, COLOR_ROW_SEL.a);
            SDL_Rect bg = {20, y, windowW - 40, rowH};
            SDL_RenderFillRect(renderer_, &bg);
        }

        int x = 24;
        int iconY = y + (ROW_HEIGHT - ICON_SIZE) / 2;  // icon 垂直居中
        SDL_Texture *iconTex = getIconTexture(games[i].iconPath);
        if (iconTex) {
            SDL_Rect iconDst = {x, iconY, ICON_SIZE, ICON_SIZE};
            SDL_RenderCopy(renderer_, iconTex, nullptr, &iconDst);
        } else {
            SDL_SetRenderDrawColor(renderer_, COLOR_ICON_PLACEHOLDER.r, COLOR_ICON_PLACEHOLDER.g, COLOR_ICON_PLACEHOLDER.b, COLOR_ICON_PLACEHOLDER.a);
            SDL_Rect iconDst = {x, iconY, ICON_SIZE, ICON_SIZE};
            SDL_RenderFillRect(renderer_, &iconDst);
        }
        x += ICON_SIZE + ICON_TEXT_GAP;

        SDL_Surface *surf = TTF_RenderUTF8_Blended(font_, games[i].name.c_str(), color);
        if (surf) {
            int textW = surf->w, textH = surf->h;
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
            SDL_FreeSurface(surf);
            if (tex) {
                int textY = y + (ROW_HEIGHT - textH) / 2;  // 文字垂直居中
                if (textY < y) textY = y;
                SDL_Rect dst = {x, textY, textW, textH};
                SDL_RenderCopy(renderer_, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
        }
    }

    SDL_RenderPresent(renderer_);
}

int MenuUI::run(const std::vector<GameEntry> &games) {
    if (!window_ || !renderer_ || !font_ || games.empty())
        return -1;

    int selected = 0;
    int count = static_cast<int>(games.size());
    leftStickY_ = 0;

    // 掌机按键：0=B 1=A 10=MENU
    static constexpr Uint8 BTN_B = 0;
    static constexpr Uint8 BTN_A = 1;
    static constexpr Uint8 BTN_MENU = 10;

    while (true) {
        bool confirm = false;
        bool quit = false;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }
            if (e.type == SDL_JOYDEVICEADDED) {
                std::fprintf(stderr, "[Launcher] JOYDEVICEADDED index=%d\n",
                             static_cast<int>(e.jdevice.which));
                handleDeviceAdded(static_cast<int>(e.jdevice.which));
            } else if (e.type == SDL_JOYDEVICEREMOVED) {
                std::fprintf(stderr, "[Launcher] JOYDEVICEREMOVED instance=%d\n",
                             static_cast<int>(e.jdevice.which));
                handleDeviceRemoved(static_cast<Sint32>(e.jdevice.which));
            } else if (e.type == SDL_CONTROLLERDEVICEADDED) {
                std::fprintf(stderr, "[Launcher] CONTROLLERDEVICEADDED index=%d\n",
                             static_cast<int>(e.cdevice.which));
                handleDeviceAdded(static_cast<int>(e.cdevice.which));
            } else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                std::fprintf(stderr, "[Launcher] CONTROLLERDEVICEREMOVED instance=%d\n",
                             static_cast<int>(e.cdevice.which));
                handleDeviceRemoved(static_cast<Sint32>(e.cdevice.which));
            } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                {
                    const char *btnName = SDL_GameControllerGetStringForButton(
                        static_cast<SDL_GameControllerButton>(e.cbutton.button));
                    std::fprintf(stderr, "[Launcher] CONTROLLERBUTTONDOWN button=%d(%s)\n",
                                 e.cbutton.button, btnName ? btnName : "?");
                }
                if (e.cbutton.button == BTN_MENU) {
                    quit = true;
                    break;
                }
                if (!quit)
                    handleControllerButton(e.cbutton.button, selected, count, confirm, quit);
            } else if (e.type == SDL_KEYDOWN) {
                std::fprintf(stderr, "[Launcher] KEYDOWN sym=%d\n",
                             static_cast<int>(e.key.keysym.sym));
                handleKey(e.key.keysym.sym, selected, count, confirm, quit);
            } else if (e.type == SDL_CONTROLLERAXISMOTION) {
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if (e.caxis.value < -AXIS_DEADZONE || e.caxis.value > AXIS_DEADZONE)
                        std::fprintf(stderr, "[Launcher] CONTROLLERAXIS LEFTY=%d\n",
                                     static_cast<int>(e.caxis.value));
                    handleControllerAxis(e.caxis.value, selected, count);
                }
            } else if (e.type == SDL_JOYAXISMOTION) {
                Sint16 v = static_cast<Sint16>(e.jaxis.value);
                if (e.jaxis.axis == 1 && (v < -AXIS_DEADZONE || v > AXIS_DEADZONE))
                    std::fprintf(stderr, "[Launcher] JOYAXIS axis=%d value=%d\n",
                                 e.jaxis.axis, static_cast<int>(v));
                handleJoyAxis(e.jaxis.axis, v, selected, count);
            } else if (e.type == SDL_JOYHATMOTION) {
                if (e.jhat.value != SDL_HAT_CENTERED)
                    std::fprintf(stderr, "[Launcher] JOYHAT value=0x%02x\n", e.jhat.value);
                handleJoyHat(e.jhat.value, selected, count);
            } else if (e.type == SDL_JOYBUTTONDOWN) {
                int btn = e.jbutton.button;
                std::fprintf(stderr, "[Launcher] JOYBUTTONDOWN button=%d\n", btn);
                if (btn == BTN_MENU) quit = true;
                else if (btn == BTN_A || btn == BTN_B) {
                    confirm = true;
                    std::fprintf(stderr, "[Launcher] CONFIRM by joy button=%d selected=%d/%d\n",
                                 btn, selected, count);
                }
            }
        }

        if (quit) {
            std::fprintf(stderr, "[Launcher] EXIT menu (quit=true)\n");
            return -1;
        }
        if (confirm) {
            const char *name = (selected >= 0 && selected < count) ? games[selected].name.c_str() : "?";
            std::fprintf(stderr, "[Launcher] RETURN selected=%d name=%s path=%s\n",
                         selected, name,
                         (selected >= 0 && selected < count) ? games[selected].path.c_str() : "?");
            return selected;
        }

        render(games, selected);
        SDL_Delay(16);
    }
}
