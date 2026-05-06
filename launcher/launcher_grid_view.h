#ifndef LAUNCHER_GRID_VIEW_H
#define LAUNCHER_GRID_VIEW_H

#include "game_list.h"
#include "launcher_theme.h"

#include <SDL.h>
#include <vector>

class LauncherTextureCache;

/**
 * 4x2 游戏网格：图标为主，名称在图标下方；超出两行时按行滚动。
 */
class LauncherGridView {
public:
    bool render(SDL_Renderer *renderer,
                LauncherTextureCache &cache,
                const std::vector<GameEntry> &games,
                int selected,
                int windowW,
                int windowH,
                const SDL_Rect &listClip);

    void resetScroll() { scrollRow_ = 0; }

private:
    int scrollRow_ = 0;
};

#endif
