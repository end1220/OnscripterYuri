#ifndef LAUNCHER_GAME_LIST_VIEW_VERTICAL_H
#define LAUNCHER_GAME_LIST_VIEW_VERTICAL_H

#include "game_list.h"
#include "launcher_theme.h"

#include <SDL.h>
#include <vector>

class LauncherTextureCache;

/**
 * 纵向游戏列表：滚动偏移与行绘制（序号、图标、名称）。
 * 坐标在 SDL_RenderSetLogicalSize 的逻辑分辨率下使用。
 */
class VerticalGameListView {
public:
    /** @return false 表示可见区间异常（与原 menu_ui startIndex>endIndex 分支一致，由调用方换底栏文案并 Present） */
    bool render(SDL_Renderer *renderer,
                LauncherTextureCache &cache,
                const std::vector<GameEntry> &games,
                int selected,
                int windowW,
                int windowH,
                const SDL_Rect &listClip);

    void resetScroll() { scrollOffset_ = 0; }

private:
    int scrollOffset_ = 0;
};

#endif
