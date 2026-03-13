#include "menu_ui.h"

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

    // 普通 SDL2 窗口，由 SDL 自己决定底层使用 GLES 还是软件渲染
    window_ = SDL_CreateWindow(
        "ONS Launcher",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN);
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

void MenuUI::shutdown() {
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
        if (selected > 0) --selected;
        else selected = count - 1;
        break;
    case SDLK_DOWN:
        if (selected < count - 1) ++selected;
        else selected = 0;
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
        confirm = true;
        break;
    case SDL_CONTROLLER_BUTTON_B:
    case SDL_CONTROLLER_BUTTON_BACK:
        quit = true;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        if (selected > 0) --selected;
        else selected = count - 1;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        if (selected < count - 1) ++selected;
        else selected = 0;
        break;
    default:
        break;
    }
}

void MenuUI::handleControllerAxis(Sint16 value, int &selected, int count) {
    if (value < -AXIS_DEADZONE) {
        if (leftStickY_ >= -AXIS_DEADZONE) {
            if (selected > 0) --selected;
            else selected = count - 1;
        }
        leftStickY_ = value;
    } else if (value > AXIS_DEADZONE) {
        if (leftStickY_ <= AXIS_DEADZONE) {
            if (selected < count - 1) ++selected;
            else selected = 0;
        }
        leftStickY_ = value;
    } else {
        leftStickY_ = 0;
    }
}

void MenuUI::render(const std::vector<GameEntry> &games, int selected) {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    int windowW = 0, windowH = 0;
    SDL_GetRendererOutputSize(renderer_, &windowW, &windowH);
    int y = 40;
    const int lineSpacing = 40;

    for (std::size_t i = 0; i < games.size(); ++i) {
        SDL_Color color = {200, 200, 200, 255};
        if (static_cast<int>(i) == selected) color = {255, 255, 0, 255};

        SDL_Surface *surf = TTF_RenderUTF8_Blended(font_, games[i].name.c_str(), color);
        if (!surf) continue;
        int textW = surf->w, textH = surf->h;
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
        SDL_FreeSurface(surf);
        if (!tex) continue;

        if (static_cast<int>(i) == selected) {
            SDL_SetRenderDrawColor(renderer_, 60, 60, 60, 255);
            SDL_Rect bg = {20, y - 5, windowW - 40, textH + 10};
            SDL_RenderFillRect(renderer_, &bg);
        }

        SDL_Rect dst = {40, y, textW, textH};
        SDL_RenderCopy(renderer_, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);

        y += lineSpacing;
        if (y > windowH - lineSpacing) break;
    }

    SDL_RenderPresent(renderer_);
}

int MenuUI::run(const std::vector<GameEntry> &games) {
    if (!window_ || !renderer_ || !font_ || games.empty())
        return -1;

    int selected = 0;
    int count = static_cast<int>(games.size());
    leftStickY_ = 0;

    while (true) {
        bool confirm = false;
        bool quit = false;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                handleKey(e.key.keysym.sym, selected, count, confirm, quit);
            } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                handleControllerButton(e.cbutton.button, selected, count, confirm, quit);
            } else if (e.type == SDL_CONTROLLERAXISMOTION) {
                if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
                    handleControllerAxis(e.caxis.value, selected, count);
            }
        }

        if (quit) return -1;
        if (confirm) return selected;

        render(games, selected);
        SDL_Delay(16);
    }
}
