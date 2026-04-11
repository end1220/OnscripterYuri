#ifndef LAUNCHER_SWITCH_GAME_ROW_VIEW_H
#define LAUNCHER_SWITCH_GAME_ROW_VIEW_H

#include "game_list.h"
#include "launcher_theme.h"

#include <SDL.h>

class LauncherTextureCache;

/**
 * Switch 式横向游戏行：逻辑坐标；tile 左起排列，scrollOffset 仿竖版仅在选择超出视口时滚动。
 */
class SwitchGameRowView {
public:
    bool render(SDL_Renderer *renderer,
                LauncherTextureCache &cache,
                const std::vector<GameEntry> &games,
                int selected,
                int windowW,
                int windowH,
                const SDL_Rect &listClip);

    void resetScroll() { scrollOffset_ = 0; }

private:
    static float scaleForDistance(int d);

    /** 水平滚动（文档坐标原点与 tile0 左对齐，与竖版 scrollOffset 语义一致） */
    int scrollOffset_ = 0;
};

#endif
