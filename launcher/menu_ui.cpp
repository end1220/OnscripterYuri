#include "menu_ui.h"
#include "launcher_theme.h"
#include "resource_loader.h"

#include <SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

bool MenuUI::shouldIgnoreJoystickInputEvent(const SDL_Event &event) const {
    if (!controller_)
        return false;

    switch (event.type) {
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP: {
        // 某些掌机会把同一物理键同时上报为 Controller + Joystick。
        // 仅屏蔽易冲突的 A/B（0/1）重复事件，保留 Menu(10) 等专用按键。
        const Uint8 btn = event.jbutton.button;
        return (btn == 0 || btn == 1);
    }
    case SDL_JOYAXISMOTION:
    case SDL_JOYHATMOTION:
        return true;
    default:
        return false;
    }
}

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

    // 使用线性过滤缩放纹理，减轻圆角遮罩边缘锯齿。
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
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

int MenuUI::run(const std::vector<GameEntry> &games) {
    if (!window_ || !renderer_ || !font_)
        return -1;

    int selected = 0;
    int count = static_cast<int>(games.size());
    mainScene_.reset(new MainScene());
    settingsScene_.reset(new SettingsScene(*mainScene_));
    sceneManager_.reset(new SceneManager(*mainScene_, *settingsScene_));

    while (true) {
        LauncherSceneAction action;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                action.quit = true;
                break;
            }
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                action.quit = true;
                break;
            }
            if (e.type == SDL_APP_WILLENTERBACKGROUND) {
                action.quit = true;
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
            } else if (shouldIgnoreJoystickInputEvent(e)) {
                continue;
            } else {
                LauncherSceneInputContext inputCtx = {games, selected, count, action};
                sceneManager_->currentScene().handleEvent(e, inputCtx);
            }
        }

        if (action.switchTo == LauncherSceneTarget::kSettings) {
            LauncherSceneRenderContext captureCtx(renderer_, font_, titleFont_, textureCache_, games,
                                                  selected);
            settingsScene_->captureBlurredBackground(captureCtx);
        }

        if (action.switchTo != LauncherSceneTarget::kNone)
            sceneManager_->switchTo(action.switchTo);

        if (action.quit)
            return -1;
        if (action.confirm)
            return selected;

        LauncherSceneRenderContext renderCtx(renderer_, font_, titleFont_, textureCache_, games,
                                             selected);
        sceneManager_->currentScene().draw(renderCtx);
        SDL_RenderPresent(renderer_);
        SDL_Delay(50);
    }
}
