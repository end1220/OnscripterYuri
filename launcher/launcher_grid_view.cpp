#include "launcher_grid_view.h"
#include "launcher_texture_cache.h"

#include <algorithm>
#include <cmath>
#include <string>

static void fillRoundedRect(SDL_Renderer *renderer, const SDL_Rect &rect, int radius) {
    if (radius <= 0 || rect.w <= 0 || rect.h <= 0) {
        SDL_RenderFillRect(renderer, &rect);
        return;
    }

    const int w = rect.w;
    const int h = rect.h;
    int r = radius;
    r = std::min(r, w / 2);
    r = std::min(r, h / 2);
    if (r <= 0) {
        SDL_RenderFillRect(renderer, &rect);
        return;
    }

    // 扫描线填充：每一行计算左右边界（圆角由四分之一圆决定）。
    for (int yy = 0; yy < h; ++yy) {
        int xLeft = rect.x;
        int xRight = rect.x + w - 1;

        if (yy < r) {
            // 距离左上角圆弧底边的垂直距离（yy=r-1 时为 0）
            const int dy = (r - 1 - yy);
            const int dx = static_cast<int>(std::floor(std::sqrt(static_cast<double>(r * r - dy * dy))));
            xLeft = rect.x + (r - dx);
            xRight = rect.x + w - 1 - (r - dx);
        } else if (yy >= h - r) {
            const int dy = (yy - (h - r));
            const int dx = static_cast<int>(std::floor(std::sqrt(static_cast<double>(r * r - dy * dy))));
            xLeft = std::max(xLeft, rect.x + (r - dx));
            xRight = std::min(xRight, rect.x + w - 1 - (r - dx));
        }

        if (xRight >= xLeft) {
            SDL_RenderDrawLine(renderer, xLeft, rect.y + yy, xRight, rect.y + yy);
        }
    }
}

bool LauncherGridView::render(SDL_Renderer *renderer,
                              LauncherTextureCache &cache,
                              const std::vector<GameEntry> &games,
                              int selected,
                              int windowW,
                              int windowH,
                              const SDL_Rect &listClip) {
    (void)listClip;
    using Grid = LauncherTheme::Grid;

    const int count = static_cast<int>(games.size());
    if (count <= 0)
        return false;

    const int rowCount = std::max(1, (count + Grid::kCols - 1) / Grid::kCols);
    const int maxScrollRow = std::max(0, rowCount - Grid::kRows);
    const int selectedRow = selected / Grid::kCols;
    if (selectedRow < scrollRow_)
        scrollRow_ = selectedRow;
    else if (selectedRow >= scrollRow_ + Grid::kRows)
        scrollRow_ = selectedRow - Grid::kRows + 1;
    if (scrollRow_ < 0)
        scrollRow_ = 0;
    if (scrollRow_ > maxScrollRow)
        scrollRow_ = maxScrollRow;

    const int gridW = Grid::kCols * Grid::kCellWidth;
    int gridLeft = (windowW - gridW) / 2;
    if (gridLeft < 0)
        gridLeft = Grid::kGridLeftPad;

    const int gridTop = Grid::kGridTopPad;
    const int nameMaxW = Grid::kCellWidth - 2 * Grid::kCellNamePadding;

    for (int visibleRow = 0; visibleRow < Grid::kRows; ++visibleRow) {
        int row = scrollRow_ + visibleRow;
        for (int col = 0; col < Grid::kCols; ++col) {
            int index = row * Grid::kCols + col;
            int cellLeft = gridLeft + col * Grid::kCellWidth;
            int cellTop = gridTop + visibleRow * Grid::kCellHeight;
            int iconX = cellLeft + (Grid::kCellWidth - Grid::kIconSize) / 2;
            int iconY = cellTop + Grid::kCellContentTopPad;
            bool isGame = index < count;
            bool isSelected = isGame && index == selected;

            if (isSelected) {
                SDL_Color bg = LauncherTheme::itemBgSelected();
                SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
                SDL_Rect cellBg = {cellLeft + Grid::kSelectedBgInset,
                                   cellTop - Grid::kSelectedBgInset,
                                   Grid::kCellWidth - 2 * Grid::kSelectedBgInset,
                                   Grid::kCellHeight - 2 * Grid::kSelectedBgInset};
                SDL_Texture *cellLightTex =
                    cache.getIconTexture(LauncherTheme::kGridSelectedBgImage);
                if (cellLightTex) {
                    SDL_RenderCopy(renderer, cellLightTex, nullptr, &cellBg);
                } else {
                    fillRoundedRect(renderer, cellBg, Grid::kSelectedBgRadius);
                }
            }

            if (isGame) {
                SDL_Texture *iconTex = cache.getMaskedIconTexture(games[index].iconPath,
                                                                  LauncherTheme::kGridIconMaskImage,
                                                                  Grid::kIconSize);
                SDL_Rect iconDst = {iconX, iconY, Grid::kIconSize, Grid::kIconSize};
                if (iconTex) {
                    SDL_RenderCopy(renderer, iconTex, nullptr, &iconDst);
                } else {
                    SDL_Color ph = LauncherTheme::iconPlaceholder();
                    SDL_SetRenderDrawColor(renderer, ph.r, ph.g, ph.b, ph.a);
                    SDL_RenderFillRect(renderer, &iconDst);
                }

                SDL_Color color =
                    isSelected ? LauncherTheme::gameNameSelected() : LauncherTheme::gameName();
                int textW = 0;
                int textH = 0;
                const std::string &nameText = games[index].name;
                std::string nameKey = std::string("grid_name:") + nameText + ":" +
                                      std::to_string(static_cast<int>(color.r)) + "," +
                                      std::to_string(static_cast<int>(color.g)) + "," +
                                      std::to_string(static_cast<int>(color.b)) + "," +
                                      std::to_string(static_cast<int>(color.a));
                SDL_Texture *nameTex =
                    cache.getTextTexture(nameKey, nameText, color, textW, textH);
                if (nameTex && textW > 0 && textH > 0) {
                    int drawW = textW;
                    int drawH = textH;
                    if (drawW > nameMaxW) {
                        drawW = nameMaxW;
                        drawH = static_cast<int>(static_cast<double>(textH) * nameMaxW / textW);
                        if (drawH < 1)
                            drawH = 1;
                    }
                    int textX = cellLeft + (Grid::kCellWidth - drawW) / 2;
                    int textY = iconY + Grid::kIconSize + Grid::kIconNameGap;
                    SDL_Rect nameDst = {textX, textY, drawW, drawH};
                    SDL_RenderCopy(renderer, nameTex, nullptr, &nameDst);
                }
            }
        }
    }

    return true;
}
