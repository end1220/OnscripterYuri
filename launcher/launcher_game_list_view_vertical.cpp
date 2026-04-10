#include "launcher_game_list_view_vertical.h"
#include "launcher_texture_cache.h"

#include <cstdio>
#include <string>

bool VerticalGameListView::render(SDL_Renderer *renderer,
                                  LauncherTextureCache &cache,
                                  const std::vector<GameEntry> &games,
                                  int selected,
                                  int windowW,
                                  int windowH,
                                  const SDL_Rect & /* listClip */) {
    using List = LauncherTheme::List;
    const int kSlotHeight = List::kSlotHeight;
    const int ROW_HEIGHT = List::kRowHeight;
    const int ROW_GAP = List::kRowGap;
    const int ICON_SIZE = List::kIconSize;
    const int ICON_TEXT_GAP = List::kIconTextGap;
    const int INDEX_LEFT = List::kIndexLeft;
    const int INDEX_ICON_GAP = List::kIndexIconGap;

    const int listTop = LauncherTheme::kListTopMargin;
    const int listBottomMargin = LauncherTheme::kListBottomMargin;
    int visibleBottom = windowH - listBottomMargin;
    int visibleHeight = visibleBottom - listTop;
    if (visibleHeight < 0)
        visibleHeight = 0;

    int totalHeight = LauncherTheme::kListTopMargin +
                      static_cast<int>(games.size()) * kSlotHeight - ROW_GAP + List::kTotalHeightTail;
    int maxScroll = (totalHeight > visibleBottom) ? (totalHeight - visibleBottom) : 0;
    int itemTop = LauncherTheme::kListTopMargin + selected * kSlotHeight;
    int itemBottom = LauncherTheme::kListTopMargin + selected * kSlotHeight + ROW_HEIGHT;
    if (itemTop - scrollOffset_ < LauncherTheme::kListTopMargin)
        scrollOffset_ = itemTop - LauncherTheme::kListTopMargin;
    else if (itemBottom - scrollOffset_ > windowH - List::kScrollItemBottomMargin)
        scrollOffset_ = itemBottom - (windowH - List::kScrollItemBottomMargin);
    if (scrollOffset_ < 0)
        scrollOffset_ = 0;
    if (scrollOffset_ > maxScroll)
        scrollOffset_ = maxScroll;

    const int baseY = LauncherTheme::kListTopMargin;
    int startIndex = scrollOffset_ / kSlotHeight;
    if (startIndex > 0)
        --startIndex;
    if (startIndex < 0)
        startIndex = 0;

    int endIndex = (scrollOffset_ + visibleHeight) / kSlotHeight + 1;
    const int count = static_cast<int>(games.size());
    if (endIndex >= count)
        endIndex = count - 1;
    if (startIndex > endIndex)
        return false;

    for (int i = startIndex; i <= endIndex; ++i) {
        int y = baseY + i * kSlotHeight - scrollOffset_;
        bool isSelected = (i == selected);
        SDL_Color color = isSelected ? LauncherTheme::gameNameSelected() : LauncherTheme::gameName();

        int rowH = ROW_HEIGHT;

        {
            const SDL_Color bgColor = isSelected ? LauncherTheme::itemBgSelected() : LauncherTheme::itemBg();
            if (bgColor.a != 0) {
                SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                SDL_Rect bg = {List::kRowPadX, y, windowW - 2 * List::kRowPadX, rowH};
                SDL_RenderFillRect(renderer, &bg);
            }
        }

        char indexBuf[16];
        snprintf(indexBuf, sizeof(indexBuf), "%d", i + 1);
        int indexW = 0;
        int indexH = 0;
        std::string indexKey = std::string("idx:") + indexBuf + ":" +
                               std::to_string(static_cast<int>(color.r)) + "," +
                               std::to_string(static_cast<int>(color.g)) + "," +
                               std::to_string(static_cast<int>(color.b)) + "," +
                               std::to_string(static_cast<int>(color.a));
        SDL_Texture *idxTex = cache.getTextTexture(indexKey, indexBuf, color, indexW, indexH);
        if (idxTex && indexW > 0 && indexH > 0) {
            int indexY = y + (ROW_HEIGHT - indexH) / 2;
            SDL_Rect idxDst = {INDEX_LEFT, indexY, indexW, indexH};
            SDL_RenderCopy(renderer, idxTex, nullptr, &idxDst);
        }

        int x = INDEX_LEFT + indexW + INDEX_ICON_GAP;
        int iconY = y + (ROW_HEIGHT - ICON_SIZE) / 2;
        SDL_Texture *iconTex = cache.getIconTexture(games[i].iconPath);
        if (iconTex) {
            SDL_Rect iconDst = {x, iconY, ICON_SIZE, ICON_SIZE};
            SDL_RenderCopy(renderer, iconTex, nullptr, &iconDst);
        } else {
            SDL_Color ph = LauncherTheme::iconPlaceholder();
            SDL_SetRenderDrawColor(renderer, ph.r, ph.g, ph.b, ph.a);
            SDL_Rect iconDst = {x, iconY, ICON_SIZE, ICON_SIZE};
            SDL_RenderFillRect(renderer, &iconDst);
        }
        x += ICON_SIZE + ICON_TEXT_GAP;

        int textW = 0;
        int textH = 0;
        const std::string &nameText = games[i].name;
        std::string nameKey = std::string("name:") + nameText + ":" +
                              std::to_string(static_cast<int>(color.r)) + "," +
                              std::to_string(static_cast<int>(color.g)) + "," +
                              std::to_string(static_cast<int>(color.b)) + "," +
                              std::to_string(static_cast<int>(color.a));
        SDL_Texture *tex = cache.getTextTexture(nameKey, nameText, color, textW, textH);
        if (tex && textW > 0 && textH > 0) {
            int textY = y + (ROW_HEIGHT - textH) / 2;
            if (textY < y)
                textY = y;
            SDL_Rect dst = {x, textY, textW, textH};
            SDL_RenderCopy(renderer, tex, nullptr, &dst);
        }
    }
    return true;
}
