#ifndef LAUNCHER_THEME_H
#define LAUNCHER_THEME_H

#include <SDL.h>

/**
 * 启动器主题：颜色、字号、列表布局常量（与 menu_ui 原散落 const 对齐）。
 * 布局坐标使用 kDesignWidth x kDesignHeight 逻辑分辨率；实际输出由 SDL_RenderSetLogicalSize 统一缩放。
 */
struct LauncherTheme {
    /**
     * 编译期开关：true 使用 Switch 式横向游戏行；false 使用竖版列表。
     * 修改后需重新编译。
     */
    static constexpr bool kUseSwitchGameRowLayout = true;

    /** 设计稿基准分辨率（与默认窗口 --windowed 960 720 一致） */
    static constexpr int kDesignWidth = 960;
    static constexpr int kDesignHeight = 720;

    static const SDL_Color &bg();
    static const SDL_Color &gameName();
    static const SDL_Color &gameNameSelected();
    static const SDL_Color &itemBg();
    static const SDL_Color &itemBgSelected();
    static const SDL_Color &titleFill();
    static const SDL_Color &titleStroke();
    static const SDL_Color &emptyHint();
    static const SDL_Color &hintOps();
    static const SDL_Color &iconPlaceholder();

    static constexpr int kTitleFontSize = 50;
    static constexpr int kGameNameFontSize = 36;
    static constexpr int kListTopMargin = 70;
    static constexpr Uint8 kBgOverlayAlpha = 120;
    static constexpr int kTitleOutlinePx = 2;
    static constexpr int kListBottomMargin = 40;
    static constexpr int kBottomOpsPadding = 20;

    /** 纵向游戏列表行布局（与 menu_ui 原 static 常量一致） */
    struct List {
        static constexpr int kIconSize = 120;
        static constexpr int kIconTextGap = 16;
        static constexpr int kRowHeight = 150;
        static constexpr int kRowGap = 5;
        static constexpr int kIndexLeft = 50;
        static constexpr int kIndexIconGap = 20;
        static constexpr int kRowPadX = 20;
        static constexpr int kTotalHeightTail = 20;
        static constexpr int kScrollItemBottomMargin = 20;
        static constexpr int kSlotHeight = kRowHeight + kRowGap;
    };

    /** Switch 横向焦点行（逻辑像素，与 List 独立） */
    struct SwitchRow {
        /** 焦点 tile 边长（选中项），4:3 逻辑分辨率下略增大 */
        static constexpr int kFocusTileSize = 200;
        /** 相邻 tile 左边缘间距（与竖版 slot 概念一致，用于滚动与布局） */
        static constexpr int kCellPitch = 224;
        /** 行中心在列表区高度内的比例（0=顶，1=底），略上移以减上下留白 */
        static constexpr float kRowCenterYFrac = 0.36f;
        /** 距选中每增加 1，缩放系数减少量（下限 kMinScale） */
        static constexpr float kScaleStep = 0.14f;
        static constexpr float kMinScale = 0.48f;
        /** 非选中 tile 颜色调制（略压暗） */
        static constexpr Uint8 kDimMod = 200;
    };

    static const char *kTitleText;
    static const char *kEmptyListHint;
    /** 右下角操作提示（空列表与非空列表正常帧） */
    static const char *kBottomOpsHintDefault;
    /** startIndex > endIndex 边界分支（与原 menu_ui 一致） */
    static const char *kBottomOpsHintEmptyRange;
};

#endif
