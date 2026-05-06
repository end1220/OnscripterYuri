#include "launcher_grid_view.h"
#include "launcher_texture_cache.h"

#include <algorithm>
#include <string>

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
            int iconY = cellTop;
            bool isGame = index < count;
            bool isSelected = isGame && index == selected;

            if (isSelected) {
                SDL_Color bg = LauncherTheme::itemBgSelected();
                SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
                SDL_Rect cellBg = {cellLeft + Grid::kSelectedBgInset,
                                   cellTop - Grid::kSelectedBgInset,
                                   Grid::kCellWidth - 2 * Grid::kSelectedBgInset,
                                   Grid::kCellHeight - 2 * Grid::kSelectedBgInset};
                SDL_RenderFillRect(renderer, &cellBg);
            }

            if (isGame) {
                SDL_Texture *iconTex = cache.getIconTexture(games[index].iconPath);
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
            } else {
                SDL_Color ph = LauncherTheme::iconPlaceholder();
                SDL_SetRenderDrawColor(renderer, ph.r, ph.g, ph.b, Grid::kEmptyCellAlpha);
                SDL_Rect placeholder = {iconX, iconY, Grid::kIconSize, Grid::kIconSize};
                SDL_RenderFillRect(renderer, &placeholder);
            }
        }
    }

    return true;
}
