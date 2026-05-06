#include "menu_ui.h"
#include "launcher_theme.h"
#include "resource_loader.h"

#include <SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

bool MenuUI::init(const std::string &fontPath, const std::string &launcherDataDir, bool windowed,
                  int windowWidth, int windowHeight) {
    fontPath_ = fontPath;

    Uint32 sdlFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;

    int numDrivers = SDL_GetNumVideoDrivers();
    bool inited = false;
    if (SDL_Init(sdlFlags) == 0) {
        inited = true;
        std::fprintf(stderr, "[Launcher] Using default video driver: %s\n",
                     SDL_GetCurrentVideoDriver());
    } else {
        std::fprintf(stderr, "[Launcher] SDL_Init default failed: %s\n", SDL_GetError());
        for (int i = 0; i < numDrivers; ++i) {
            const char *drv = SDL_GetVideoDriver(i);
            if (!drv)
                continue;
            std::fprintf(stderr, "[Launcher] Try video driver: %s\n", drv);
            SDL_Quit();
            setenv("SDL_VIDEODRIVER", drv, 1);
            if (SDL_Init(sdlFlags) == 0) {
                inited = true;
                std::fprintf(stderr, "[Launcher] Using video driver: %s\n",
                             SDL_GetCurrentVideoDriver());
                break;
            }
            std::fprintf(stderr, "[Launcher] Driver \"%s\" failed: %s\n", drv, SDL_GetError());
        }
    }

    if (!inited) {
        std::fprintf(stderr, "SDL init failed: no working video driver. %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    const int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::fprintf(stderr, "IMG_Init PNG/JPG failed: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if (windowed) {
        std::fprintf(stderr, "[Launcher] Windowed mode: %dx%d\n", windowWidth, windowHeight);
        window_ = SDL_CreateWindow("ONS Launcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    } else {
        window_ = SDL_CreateWindow("ONS Launcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0,
                                   0, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    std::string pakPath = launcherDataDir.empty() ? "data.pak" : (launcherDataDir + "/data.pak");
    ResourceLoader::instance().init(pakPath);

    const char *path = fontPath_.empty() ? "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
                                         : fontPath_.c_str();
    SDL_JoystickEventState(SDL_ENABLE);
    if (SDL_NumJoysticks() > 0)
        openDevice(0);

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    // 逻辑分辨率固定为设计稿；SDL 负责缩放到实际窗口（含 letterbox），布局与字号均按逻辑坐标
    if (SDL_RenderSetLogicalSize(renderer_, LauncherTheme::kDesignWidth, LauncherTheme::kDesignHeight) !=
        0)
        std::fprintf(stderr, "[Launcher] SDL_RenderSetLogicalSize failed: %s\n", SDL_GetError());
    SDL_RenderSetIntegerScale(renderer_, SDL_FALSE);

    titleFont_ = TTF_OpenFont(path, LauncherTheme::kTitleFontSize);
    if (!titleFont_) {
        std::fprintf(stderr, "TTF_OpenFont (title) failed for %s: %s\n", path, TTF_GetError());
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    font_ = TTF_OpenFont(path, LauncherTheme::kGameNameFontSize);
    if (!font_) {
        std::fprintf(stderr, "TTF_OpenFont (game name) failed for %s: %s\n", path, TTF_GetError());
        TTF_CloseFont(titleFont_);
        titleFont_ = nullptr;
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    textureCache_.init(renderer_, font_);
    return true;
}

void MenuUI::openDevice(int index) {
    closeDevice();
    if (SDL_IsGameController(index)) {
        controller_ = SDL_GameControllerOpen(index);
        (void)controller_;
    } else {
        joystick_ = SDL_JoystickOpen(index);
        (void)joystick_;
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
        if (j)
            ourId = SDL_JoystickInstanceID(j);
    } else if (joystick_) {
        ourId = SDL_JoystickInstanceID(joystick_);
    }
    if (ourId >= 0 && ourId == instanceId)
        closeDevice();
}

void MenuUI::shutdown() {
    closeDevice();
    textureCache_.clear();
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (titleFont_) {
        TTF_CloseFont(titleFont_);
        titleFont_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    ResourceLoader::instance().shutdown();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void MenuUI::render(const std::vector<GameEntry> &games, int selected) {
    int windowW = LauncherTheme::kDesignWidth;
    int windowH = LauncherTheme::kDesignHeight;
    SDL_RenderGetLogicalSize(renderer_, &windowW, &windowH);
    if (windowW <= 0 || windowH <= 0) {
        windowW = LauncherTheme::kDesignWidth;
        windowH = LauncherTheme::kDesignHeight;
    }

    chrome_.clearBackground(renderer_, windowW, windowH);
    chrome_.drawBackgroundOverlay(textureCache_, renderer_, windowW, windowH);
    chrome_.drawTitle(titleFont_, renderer_, windowW);

    const int listTop = LauncherTheme::kListTopMargin;
    const int listBottomMargin = LauncherTheme::kListBottomMargin;
    int listHeight = windowH - listTop - listBottomMargin;
    if (listHeight < 0)
        listHeight = 0;
    SDL_Rect listClip = {0, listTop, windowW, listHeight};
    SDL_RenderSetClipRect(renderer_, &listClip);

    const char *bottomHint = (LauncherTheme::kLayout == LauncherTheme::Layout::kGrid)
                                 ? LauncherTheme::kBottomOpsHintGrid
                                 : LauncherTheme::kBottomOpsHintDefault;

    if (games.empty()) {
        chrome_.drawEmptyListHint(font_, renderer_, windowW, windowH);
        SDL_RenderSetClipRect(renderer_, nullptr);
        chrome_.drawBottomOpsHint(font_, renderer_, windowW, windowH, bottomHint);
        SDL_RenderPresent(renderer_);
        return;
    }

    bool drewList = false;
    switch (LauncherTheme::kLayout) {
    case LauncherTheme::Layout::kGrid:
        drewList = gridView_.render(renderer_, textureCache_, games, selected, windowW, windowH,
                                    listClip);
        break;
    case LauncherTheme::Layout::kSwitchRow:
        drewList = switchRowView_.render(renderer_, textureCache_, games, selected, windowW, windowH,
                                         listClip);
        break;
    case LauncherTheme::Layout::kVertical:
    default:
        drewList = listView_.render(renderer_, textureCache_, games, selected, windowW, windowH,
                                      listClip);
        break;
    }
    SDL_RenderSetClipRect(renderer_, nullptr);

    if (!drewList) {
        chrome_.drawBottomOpsHint(font_, renderer_, windowW, windowH,
                                  LauncherTheme::kBottomOpsHintEmptyRange);
        SDL_RenderPresent(renderer_);
        return;
    }

    chrome_.drawBottomOpsHint(font_, renderer_, windowW, windowH, bottomHint);
    SDL_RenderPresent(renderer_);
}

int MenuUI::run(const std::vector<GameEntry> &games) {
    if (!window_ || !renderer_ || !font_)
        return -1;

    int selected = 0;
    int count = static_cast<int>(games.size());
    menuInput_.resetStickState();

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
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                quit = true;
                break;
            }
            if (e.type == SDL_APP_WILLENTERBACKGROUND) {
                quit = true;
                break;
            }
            if (e.type == SDL_JOYDEVICEADDED) {
                handleDeviceAdded(static_cast<int>(e.jdevice.which));
            } else if (e.type == SDL_JOYDEVICEREMOVED) {
                handleDeviceRemoved(static_cast<Sint32>(e.jdevice.which));
            } else if (e.type == SDL_CONTROLLERDEVICEADDED) {
                handleDeviceAdded(static_cast<int>(e.cdevice.which));
            } else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                handleDeviceRemoved(static_cast<Sint32>(e.cdevice.which));
            } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                if (e.cbutton.button == BTN_MENU) {
                    quit = true;
                    break;
                }
                if (!quit)
                    menuInput_.handleControllerButton(e.cbutton.button, selected, count, confirm,
                                                      quit);
            } else if (e.type == SDL_KEYDOWN) {
                menuInput_.handleKey(e.key.keysym.sym, selected, count, confirm, quit);
            } else if (e.type == SDL_CONTROLLERAXISMOTION) {
                menuInput_.handleControllerAxisMotion(e.caxis.axis, e.caxis.value, selected, count);
            } else if (e.type == SDL_JOYAXISMOTION) {
                Sint16 v = static_cast<Sint16>(e.jaxis.value);
                menuInput_.handleJoyAxisMotion(e.jaxis.axis, v, selected, count);
            } else if (e.type == SDL_JOYHATMOTION) {
                menuInput_.handleJoyHatMotion(e.jhat.value, selected, count);
            } else if (e.type == SDL_JOYBUTTONDOWN) {
                int btn = e.jbutton.button;
                if (btn == BTN_MENU)
                    quit = true;
                else if (btn == BTN_A || btn == BTN_B) {
                    if (count > 0)
                        confirm = true;
                }
            }
        }

        if (quit)
            return -1;
        if (confirm)
            return selected;

        render(games, selected);
        SDL_Delay(50);
    }
}
