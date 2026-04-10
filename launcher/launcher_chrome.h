#ifndef LAUNCHER_CHROME_H
#define LAUNCHER_CHROME_H

#include "launcher_theme.h"

#include <SDL.h>
#include <SDL_ttf.h>

class LauncherTextureCache;

/**
 * 启动器非列表区域：清屏、背景图、顶栏标题、空列表提示、底栏操作提示。
 * 坐标应在 SDL_RenderSetLogicalSize 设定的逻辑分辨率下使用。
 */
class LauncherChrome {
public:
    void clearBackground(SDL_Renderer *renderer, int windowW, int windowH);

    /** 绘制 data/bg.jpg 叠层；绘制后将纹理 alpha 恢复为 255 */
    void drawBackgroundOverlay(LauncherTextureCache &cache, SDL_Renderer *renderer, int windowW, int windowH);

    void drawTitle(TTF_Font *titleFont, SDL_Renderer *renderer, int windowW);

    void drawEmptyListHint(TTF_Font *listFont, SDL_Renderer *renderer, int windowW, int windowH);

    void drawBottomOpsHint(TTF_Font *listFont, SDL_Renderer *renderer, int windowW, int windowH,
                           const char *hintOps, float scale = 1.0f);
};

#endif
