#include "menu_ui.h"

#include <SDL_image.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Launcher 主题颜色与布局/字号配置（仅在本实现文件内使用）
const SDL_Color LAUNCHER_COLOR_BG            = {40, 100, 130, 255};    // 背景底色（纯色）
const SDL_Color LAUNCHER_COLOR_GAME_NAME     = {225, 240, 245, 255}; // 未选中：更接近白
const SDL_Color LAUNCHER_COLOR_GAME_NAME_SEL = {66, 225, 205, 255};   // 选中：更亮的青绿
const SDL_Color LAUNCHER_COLOR_ITEM_BG       = {0, 235, 205, 0};    // 未选中底图：更亮一点
const SDL_Color LAUNCHER_COLOR_ITEM_BG_SEL   = {235, 235, 235, 120};   // 选中底图：对比更明显
const SDL_Color LAUNCHER_COLOR_TITLE         = {245, 235, 245, 255}; // 标题填充：几乎纯白
const SDL_Color LAUNCHER_COLOR_TITLE_STROKE  = {0, 90, 120, 255};   // 标题描边：和高亮一致
const SDL_Color LAUNCHER_COLOR_EMPTY_HINT    = {170, 190, 200, 255}; // 空目录提示：略亮灰蓝
const SDL_Color LAUNCHER_COLOR_HINT_OPS      = {185, 205, 215, 255}; // 右下提示：再亮一点的灰蓝

// 启动器纯色背景上叠加的半透明背景图（bg.jpg）的整体透明度
const Uint8 LAUNCHER_BG_OVERLAY_ALPHA = 96; // 取值 0~255，值越小越透明

// 字号
const int LAUNCHER_TITLE_FONT_SIZE = 50;       // 顶部标题字号（稍大）
const int LAUNCHER_GAME_NAME_FONT_SIZE = 36;   // 游戏名称字号

// 布局
const int LAUNCHER_LIST_TOP_MARGIN = 70;       // 游戏列表可视区域上边界（在标题下方）

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
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    // 同时初始化 PNG 与 JPG，以便加载 bg.jpg 等背景图片
    const int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::fprintf(stderr, "IMG_Init PNG/JPG failed: %s\n", IMG_GetError());
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

    // 启用 alpha 混合，以便条目底图的透明度生效
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    // 顶部标题字体
    titleFont_ = TTF_OpenFont(path, LAUNCHER_TITLE_FONT_SIZE);
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

    // 游戏列表字体
    font_ = TTF_OpenFont(path, LAUNCHER_GAME_NAME_FONT_SIZE);
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
    for (auto &p : textCache_) {
        if (p.second.texture) SDL_DestroyTexture(p.second.texture);
    }
    textCache_.clear();
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
    // 确保纹理使用 alpha 混合，这样 SDL_SetTextureAlphaMod 才能生效
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    iconCache_[iconPath] = tex;
    return tex;
}

SDL_Texture *MenuUI::getTextTexture(const std::string &cacheKey,
                                    const std::string &text,
                                    const SDL_Color &color,
                                    int &outW,
                                    int &outH) {
    auto it = textCache_.find(cacheKey);
    if (it != textCache_.end()) {
        outW = it->second.width;
        outH = it->second.height;
        return it->second.texture;
    }

    if (!font_) {
        outW = outH = 0;
        return nullptr;
    }

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
    if (!surf) {
        outW = outH = 0;
        return nullptr;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        outW = outH = 0;
        return nullptr;
    }

    TextCacheEntry entry;
    entry.texture = tex;
    entry.width = surf->w;
    entry.height = surf->h;
    textCache_[cacheKey] = entry;

    outW = entry.width;
    outH = entry.height;

    SDL_FreeSurface(surf);
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
static const int ICON_SIZE = 120;    // 图标尺寸
static const int ICON_TEXT_GAP = 16;
static const int ROW_HEIGHT = 150;   // 每行绘制高度（稍微减小）
static const int ROW_GAP = 5;        // 两个条目之间的间距
static const int SLOT_HEIGHT = ROW_HEIGHT + ROW_GAP;

static const SDL_Color COLOR_ICON_PLACEHOLDER = {200, 230, 235, 255};  // 无图标占位（浅青灰）

void MenuUI::render(const std::vector<GameEntry> &games, int selected) {
    SDL_SetRenderDrawColor(renderer_,
                           LAUNCHER_COLOR_BG.r,
                           LAUNCHER_COLOR_BG.g,
                           LAUNCHER_COLOR_BG.b,
                           LAUNCHER_COLOR_BG.a);
    SDL_RenderClear(renderer_);

    int windowW = 0, windowH = 0;
    SDL_GetRendererOutputSize(renderer_, &windowW, &windowH);

    // 在纯色背景上铺一张半透明的背景图（bg.jpg）
    // 图片路径相对于启动目录（例如 run_launcher.sh 切换到的工作目录）
    SDL_Texture *bgTex = getIconTexture("data/bg.jpg");
    if (bgTex) {
        SDL_SetTextureAlphaMod(bgTex, LAUNCHER_BG_OVERLAY_ALPHA);
        SDL_Rect dst = {0, 0, windowW, windowH};
        SDL_RenderCopy(renderer_, bgTex, nullptr, &dst);
    }

    // 顶部居中绘制标题 “ONS游戏”（使用描边 + 填充）
    if (titleFont_) {
        const char *titleText = "ONS游戏";

        int titleW = 0;
        int titleH = 0;

        // 1. 先渲染描边
        TTF_SetFontOutline(titleFont_, 2);  // 描边宽度
        SDL_Surface *strokeSurf = TTF_RenderUTF8_Blended(titleFont_, titleText, LAUNCHER_COLOR_TITLE_STROKE);
        if (strokeSurf) {
            SDL_Texture *strokeTex = SDL_CreateTextureFromSurface(renderer_, strokeSurf);
            if (strokeTex) {
                titleW = strokeSurf->w;
                titleH = strokeSurf->h;
                int titleX = (windowW - titleW) / 2;
                int titleY = (LAUNCHER_LIST_TOP_MARGIN - titleH) / 2;  // 在列表顶部区域内垂直居中
                SDL_Rect dst = {titleX, titleY, titleW, titleH};
                SDL_RenderCopy(renderer_, strokeTex, nullptr, &dst);
                SDL_DestroyTexture(strokeTex);
            }
            SDL_FreeSurface(strokeSurf);
        }

        // 2. 再渲染填充文字（无描边）
        TTF_SetFontOutline(titleFont_, 0);
        SDL_Surface *fillSurf = TTF_RenderUTF8_Blended(titleFont_, titleText, LAUNCHER_COLOR_TITLE);
        if (fillSurf) {
            SDL_Texture *fillTex = SDL_CreateTextureFromSurface(renderer_, fillSurf);
            if (fillTex) {
                // 使用与描边相同的尺寸和位置（若上面没成功，重新计算一次）
                if (titleW == 0 || titleH == 0) {
                    titleW = fillSurf->w;
                    titleH = fillSurf->h;
                }
                int titleX = (windowW - titleW) / 2;
                int titleY = (LAUNCHER_LIST_TOP_MARGIN - titleH) / 2;
                SDL_Rect dst = {titleX, titleY, titleW, titleH};
                SDL_RenderCopy(renderer_, fillTex, nullptr, &dst);
                SDL_DestroyTexture(fillTex);
            }
            SDL_FreeSurface(fillSurf);
        }
    }

    // 列表绘制区域裁剪，避免与顶部标题重叠，并在底部预留高度显示操作提示
    const int listTop = LAUNCHER_LIST_TOP_MARGIN;
    const int listBottomMargin = 40;  // 底部保留 40 像素给提示文字
    int listHeight = windowH - listTop - listBottomMargin;
    if (listHeight < 0) listHeight = 0;
    SDL_Rect listClip = {0, listTop, windowW, listHeight};
    SDL_RenderSetClipRect(renderer_, &listClip);

    // 若没有任何游戏条目，显示提示文字并退出本帧绘制
    if (games.empty()) {
        if (font_) {
            const char *hintText = "Roms/ONS目录里没有ONS游戏哦~";
            SDL_Surface *hintSurf = TTF_RenderUTF8_Blended(font_, hintText, LAUNCHER_COLOR_EMPTY_HINT);
            if (hintSurf) {
                SDL_Texture *hintTex = SDL_CreateTextureFromSurface(renderer_, hintSurf);
                if (hintTex) {
                    int textW = hintSurf->w;
                    int textH = hintSurf->h;
                    int textX = (windowW - textW) / 2;
                    int textY = (windowH - textH) / 2;
                    SDL_Rect dst = {textX, textY, textW, textH};
                    SDL_RenderCopy(renderer_, hintTex, nullptr, &dst);
                    SDL_DestroyTexture(hintTex);
                }
                SDL_FreeSurface(hintSurf);
            }
        }
        SDL_RenderSetClipRect(renderer_, nullptr);
        SDL_RenderPresent(renderer_);
        return;
    }

    int totalHeight = LAUNCHER_LIST_TOP_MARGIN + static_cast<int>(games.size()) * SLOT_HEIGHT - ROW_GAP + 20;
    int visibleBottom = windowH - listBottomMargin;
    int visibleHeight = visibleBottom - listTop;
    if (visibleHeight < 0) visibleHeight = 0;
    int maxScroll = (totalHeight > visibleBottom) ? (totalHeight - visibleBottom) : 0;
    int itemTop = LAUNCHER_LIST_TOP_MARGIN + selected * SLOT_HEIGHT;
    int itemBottom = LAUNCHER_LIST_TOP_MARGIN + selected * SLOT_HEIGHT + ROW_HEIGHT;
    if (itemTop - scrollOffset_ < LAUNCHER_LIST_TOP_MARGIN)
        scrollOffset_ = itemTop - LAUNCHER_LIST_TOP_MARGIN;
    else if (itemBottom - scrollOffset_ > windowH - 20)
        scrollOffset_ = itemBottom - (windowH - 20);
    if (scrollOffset_ < 0) scrollOffset_ = 0;
    if (scrollOffset_ > maxScroll) scrollOffset_ = maxScroll;

    const int baseY = LAUNCHER_LIST_TOP_MARGIN;
    const int clipTop = listTop;
    const int clipBottom = visibleBottom;
    // 只遍历可见行区间，长列表时减少循环与缓存查找次数
    // 使用非负整除，避免第一条目在 scrollOffset_ 为 0 时被跳过
    int startIndex = scrollOffset_ / SLOT_HEIGHT;
    if (startIndex > 0) --startIndex;  // 多绘制一条在可见区域上方的行，避免滚动边缘闪烁
    if (startIndex < 0) startIndex = 0;

    int endIndex = (scrollOffset_ + visibleHeight) / SLOT_HEIGHT + 1;
    const int count = static_cast<int>(games.size());
    if (endIndex >= count) endIndex = count - 1;
    if (startIndex > endIndex) {
        SDL_RenderSetClipRect(renderer_, nullptr);

        // 绘制底部操作提示
        if (font_) {
            const char *hintOps = "M-退出  A-启动游戏 ";
            SDL_Surface *opsSurf = TTF_RenderUTF8_Blended(font_, hintOps, LAUNCHER_COLOR_HINT_OPS);
            if (opsSurf) {
                SDL_Texture *opsTex = SDL_CreateTextureFromSurface(renderer_, opsSurf);
                if (opsTex) {
                    int textW = opsSurf->w;
                    int textH = opsSurf->h;
                    // 将底部操作提示整体缩小
                    const float scale = 1.0f;
                    int dstW = static_cast<int>(textW * scale);
                    int dstH = static_cast<int>(textH * scale);
                    int padding = 20;
                    int textX = windowW - dstW - padding;
                    int textY = windowH - dstH;
                    SDL_Rect dst = {textX, textY, dstW, dstH};
                    SDL_RenderCopy(renderer_, opsTex, nullptr, &dst);
                    SDL_DestroyTexture(opsTex);
                }
                SDL_FreeSurface(opsSurf);
            }
        }

        SDL_RenderPresent(renderer_);
        return;
    }

    for (int i = startIndex; i <= endIndex; ++i) {
        int y = baseY + i * SLOT_HEIGHT - scrollOffset_;
        bool isSelected = (i == selected);
        SDL_Color color = isSelected ? LAUNCHER_COLOR_GAME_NAME_SEL
                                     : LAUNCHER_COLOR_GAME_NAME;

        int rowH = ROW_HEIGHT;

        // 条目底图（未选中/选中）
        {
            const SDL_Color bgColor = isSelected ? LAUNCHER_COLOR_ITEM_BG_SEL : LAUNCHER_COLOR_ITEM_BG;
            if (bgColor.a != 0) {
                SDL_SetRenderDrawColor(renderer_, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                SDL_Rect bg = {20, y, windowW - 40, rowH};
                SDL_RenderFillRect(renderer_, &bg);
            }
        }

        // 序号：距左边 20px，与图标间隔 20px
        static const int INDEX_LEFT = 50;
        static const int INDEX_ICON_GAP = 20;
        char indexBuf[16];
        snprintf(indexBuf, sizeof(indexBuf), "%d", i + 1);
        int indexW = 0;
        int indexH = 0;
        std::string indexKey = std::string("idx:") + indexBuf +
                               ":" + std::to_string(static_cast<int>(color.r)) +
                               "," + std::to_string(static_cast<int>(color.g)) +
                               "," + std::to_string(static_cast<int>(color.b)) +
                               "," + std::to_string(static_cast<int>(color.a));
        SDL_Texture *idxTex = getTextTexture(indexKey, indexBuf, color, indexW, indexH);
        if (idxTex && indexW > 0 && indexH > 0) {
            int indexY = y + (ROW_HEIGHT - indexH) / 2;
            SDL_Rect idxDst = {INDEX_LEFT, indexY, indexW, indexH};
            SDL_RenderCopy(renderer_, idxTex, nullptr, &idxDst);
        }

        int x = INDEX_LEFT + indexW + INDEX_ICON_GAP;
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

        int textW = 0;
        int textH = 0;
        const std::string &nameText = games[i].name;
        std::string nameKey = std::string("name:") + nameText +
                              ":" + std::to_string(static_cast<int>(color.r)) +
                              "," + std::to_string(static_cast<int>(color.g)) +
                              "," + std::to_string(static_cast<int>(color.b)) +
                              "," + std::to_string(static_cast<int>(color.a));
        SDL_Texture *tex = getTextTexture(nameKey, nameText, color, textW, textH);
        if (tex && textW > 0 && textH > 0) {
            int textY = y + (ROW_HEIGHT - textH) / 2;  // 文字垂直居中
            if (textY < y) textY = y;
            SDL_Rect dst = {x, textY, textW, textH};
            SDL_RenderCopy(renderer_, tex, nullptr, &dst);
        }
    }

    SDL_RenderSetClipRect(renderer_, nullptr);

    // 绘制底部操作提示
    if (font_) {
        const char *hintOps = "M 退出  A 启动游戏";
        SDL_Surface *opsSurf = TTF_RenderUTF8_Blended(font_, hintOps, LAUNCHER_COLOR_HINT_OPS);
        if (opsSurf) {
            SDL_Texture *opsTex = SDL_CreateTextureFromSurface(renderer_, opsSurf);
            if (opsTex) {
                int textW = opsSurf->w;
                int textH = opsSurf->h;
                // 将底部操作提示整体缩小到 70%
                const float scale = 0.7f;
                int dstW = static_cast<int>(textW * scale);
                int dstH = static_cast<int>(textH * scale);
                int padding = 20;
                int textX = windowW - dstW - padding;
                int textY = windowH - dstH - 10;  // 底部稍微往上 10 像素
                SDL_Rect dst = {textX, textY, dstW, dstH};
                SDL_RenderCopy(renderer_, opsTex, nullptr, &dst);
                SDL_DestroyTexture(opsTex);
            }
            SDL_FreeSurface(opsSurf);
        }
    }

    SDL_RenderPresent(renderer_);
}

int MenuUI::run(const std::vector<GameEntry> &games) {
    if (!window_ || !renderer_ || !font_)
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
            if (e.type == SDL_WINDOWEVENT &&
                e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                std::fprintf(stderr, "[Launcher] FOCUS_LOST -> exit\n");
                quit = true;
                break;
            }
            if (e.type == SDL_APP_WILLENTERBACKGROUND) {
                std::fprintf(stderr, "[Launcher] WILLENTERBACKGROUND -> exit\n");
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
        SDL_Delay(50); // 约 20 FPS
    }
}
