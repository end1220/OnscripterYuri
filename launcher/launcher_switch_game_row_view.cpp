#include "launcher_switch_game_row_view.h"
#include "launcher_texture_cache.h"

#include <cmath>
#include <string>

float SwitchGameRowView::scaleForDistance(int d) {
    using SR = LauncherTheme::SwitchRow;
    float s = 1.f - SR::kScaleStep * static_cast<float>(d);
    if (s < SR::kMinScale)
        s = SR::kMinScale;
    return s;
}

bool SwitchGameRowView::render(SDL_Renderer *renderer,
                               LauncherTextureCache &cache,
                               const std::vector<GameEntry> &games,
                               int selected,
                               int windowW,
                               int windowH,
                               const SDL_Rect & /* listClip */) {
    using SR = LauncherTheme::SwitchRow;
    using List = LauncherTheme::List;
    const int listTop = LauncherTheme::kListTopMargin;
    const int listBottomMargin = LauncherTheme::kListBottomMargin;
    const int listH = windowH - listTop - listBottomMargin;
    if (listH <= 0)
        return false;

    const int baseX = List::kRowPadX;
    const int rightPad = List::kRowPadX;
    const int step = SR::kCellPitch;
    const int focus = SR::kFocusTileSize;
    const int n = static_cast<int>(games.size());
    if (n <= 0)
        return false;

    const int rowCenterY =
        listTop + static_cast<int>(static_cast<float>(listH) * SR::kRowCenterYFrac);

    const long totalDocWidth = static_cast<long>(n - 1) * step + focus;
    const int visibleW = windowW - baseX - rightPad;
    if (visibleW <= 0)
        return false;

    const long maxScroll =
        (totalDocWidth > visibleW) ? static_cast<long>(totalDocWidth - visibleW) : 0;

    const long docLeftSel = static_cast<long>(selected) * step;
    const long screenLeftSel = static_cast<long>(baseX) + docLeftSel - scrollOffset_;
    const long screenRightSel = screenLeftSel + focus;

    if (screenLeftSel < baseX)
        scrollOffset_ = static_cast<int>(static_cast<long>(baseX) + docLeftSel - baseX);
    else if (screenRightSel > static_cast<long>(windowW) - rightPad)
        scrollOffset_ = static_cast<int>(static_cast<long>(baseX) + docLeftSel + focus -
                                        (static_cast<long>(windowW) - rightPad));

    if (scrollOffset_ < 0)
        scrollOffset_ = 0;
    if (static_cast<long>(scrollOffset_) > maxScroll)
        scrollOffset_ = static_cast<int>(maxScroll);

    int maxDist = 0;
    for (int i = 0; i < n; ++i) {
        int d = std::abs(i - selected);
        if (d > maxDist)
            maxDist = d;
    }

    for (int dist = maxDist; dist >= 0; --dist) {
        for (int i = 0; i < n; ++i) {
            if (std::abs(i - selected) != dist)
                continue;

            float sc = scaleForDistance(dist);
            int tileSize = static_cast<int>(std::lround(SR::kFocusTileSize * sc));
            if (tileSize < 1)
                tileSize = 1;

            const long docLeft = static_cast<long>(i) * step;
            int x = static_cast<int>(static_cast<long>(baseX) + docLeft - scrollOffset_);
            int y = rowCenterY - tileSize / 2;

            bool isSel = (i == selected);
            SDL_Texture *iconTex = cache.getIconTexture(games[i].iconPath);
            if (iconTex) {
                if (!isSel)
                    SDL_SetTextureColorMod(iconTex, SR::kDimMod, SR::kDimMod, SR::kDimMod);
                SDL_Rect dst = {x, y, tileSize, tileSize};
                SDL_RenderCopy(renderer, iconTex, nullptr, &dst);
                SDL_SetTextureColorMod(iconTex, 255, 255, 255);
            } else {
                SDL_Color ph = LauncherTheme::iconPlaceholder();
                SDL_SetRenderDrawColor(renderer, ph.r, ph.g, ph.b, ph.a);
                SDL_Rect dst = {x, y, tileSize, tileSize};
                SDL_RenderFillRect(renderer, &dst);
            }
        }
    }

    const std::string &nameText = games[selected].name;
    SDL_Color nameColor = LauncherTheme::gameNameSelected();
    int textW = 0;
    int textH = 0;
    std::string nameKey = std::string("switch_name:") + nameText + ":" +
                          std::to_string(static_cast<int>(nameColor.r)) + "," +
                          std::to_string(static_cast<int>(nameColor.g)) + "," +
                          std::to_string(static_cast<int>(nameColor.b)) + "," +
                          std::to_string(static_cast<int>(nameColor.a));
    SDL_Texture *nameTex = cache.getTextTexture(nameKey, nameText, nameColor, textW, textH);
    if (nameTex && textW > 0 && textH > 0) {
        int maxW = windowW - 40;
        int drawW = textW;
        int drawH = textH;
        if (drawW > maxW) {
            drawW = maxW;
            drawH = static_cast<int>(static_cast<double>(textH) * maxW / textW);
            if (drawH < 1)
                drawH = 1;
        }
        int nameX = (windowW - drawW) / 2;
        int nameY = rowCenterY + SR::kFocusTileSize / 2 + 20;
        if (nameY + drawH > windowH - listBottomMargin - 4)
            nameY = windowH - listBottomMargin - drawH - 4;
        if (nameY < rowCenterY)
            nameY = rowCenterY + 8;
        SDL_Rect dst = {nameX, nameY, drawW, drawH};
        SDL_RenderCopy(renderer, nameTex, nullptr, &dst);
    }

    return true;
}
