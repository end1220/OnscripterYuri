#ifndef LAUNCHER_THEME_H
#define LAUNCHER_THEME_H

#include <SDL.h>

/**
 * 启动器主题：颜色、字号、列表布局常量（与 menu_ui 原散落 const 对齐）。
 * 布局坐标使用 kDesignWidth x kDesignHeight 逻辑分辨率；实际输出由 SDL_RenderSetLogicalSize 统一缩放。
 */
struct LauncherTheme {
    /**
     * 编译期布局开关；修改后需重新编译。
     * kUseSwitchGameRowLayout 保留给旧分支判断使用。
     * 修改后需重新编译。
     */
    enum class Layout { kVertical, kSwitchRow, kGrid };
    static constexpr Layout kLayout = Layout::kGrid;
    static constexpr bool kUseSwitchGameRowLayout = (kLayout == Layout::kSwitchRow);

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
    static const SDL_Color &settingsBackdrop();
    static const SDL_Color &settingsPanel();
    static const SDL_Color &settingsPanelBorder();
    static const SDL_Color &settingsButton();
    static const SDL_Color &settingsButtonFocused();
    static const SDL_Color &settingsText();

    static constexpr int kTitleFontSize = 50;
    static constexpr int kGameNameFontSize = 26;
    static constexpr int kListTopMargin = 70;
    static constexpr Uint8 kBgOverlayAlpha = 100;
    static constexpr int kTitleOutlinePx = 2;
    static constexpr int kListBottomMargin = 40;
    static constexpr int kBottomOpsPadding = 20;
    static constexpr int kSettingsPanelSize = 420;
    static constexpr float kSettingsPanelSizeRatio = 0.75f;
    static constexpr int kSettingsPanelBorderPx = 2;
    static constexpr int kSettingsButtonWidth = 220;
    static constexpr int kSettingsButtonHeight = 64;
    static constexpr int kSettingsButtonTopOffset = 146;
    static constexpr int kSettingsSignatureBottomOffset = 64;
    /**
     * 设置页顶栏「菜单」与左侧按钮相对 listFont 的缩放。
     * 与修改前相比：顶栏与左侧按钮默认字号数值已对调（原顶栏 1.0 ↔ 原按钮未选中 1.1）。
     */
    static constexpr float kSettingsSignatureTextScale = 1.1f;
    static constexpr float kSettingsLeftButtonScaleNormal = 1.0f;
    static constexpr float kSettingsLeftButtonScaleSelected = 1.2f;

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

    /** 4x2 网格布局（逻辑像素，面向 960x720 设计分辨率） */
    struct Grid {
        static constexpr int kCols = 4;
        static constexpr int kRows = 2;
        static constexpr int kCellWidth = 220;
        static constexpr int kCellHeight = 290;
        static constexpr int kGridLeftPad = (kDesignWidth - kCols * kCellWidth) / 2;
        static constexpr int kGridTopPad = kListTopMargin + 12;
        /** Cell 内容（icon + name）整体向下的内边距 */
        static constexpr int kCellContentTopPad = 20;
        static constexpr int kIconSize = 160;
        static constexpr int kIconNameGap = 12;
        static constexpr int kSelectedBgInset = 6;
        static constexpr int kSelectedBgRadius = 18;
        static constexpr int kCellNamePadding = 10;
        static constexpr Uint8 kEmptyCellAlpha = 90;
    };

    static const char *kTitleText;
    static const char *kEmptyListHint;
    /** 右下角操作提示（空列表与非空列表正常帧） */
    static const char *kBottomOpsHintDefault;
    /** Grid 布局右下角操作提示 */
    static const char *kBottomOpsHintGrid;
    static const char *kSettingsExitText;
    static const char *kSettingsReturnText;
    static const char *kSettingsAuthorTabText;
    static const char *kSettingsAboutTabText;
    static const char *kSettingsAuthorNameText;
    static const char *kSettingsAuthorLinkText;
    /** 设置页右侧默认说明（返 回 / 关 于 / 退 出 等与「作者」并列项共用；「作者」为简介） */
    static const char *kSettingsHelpTitleText;
    static const char *kSettingsHelpLine1Text;
    static const char *kSettingsHelpLine3Text;
    /** kSettingsHelpLine3Text 续行（避免单行过长超出右侧面板） */
    static const char *kSettingsHelpLine3PointerText;
    static const char *kSettingsHelpLine5Text;
    /** 选中「关 于」时右侧说明区最上方一行 */
    static const char *kSettingsAboutDeviceBannerText;
    static const char *kSettingsHelpLine6Text;
    static const char *kSettingsSignatureText;
    /** Grid 选中高亮背景图（pak 逻辑路径需带 data/ 前缀） */
    static const char *kGridSelectedBgImage;
    /** Grid 图标圆角遮罩图（pak 逻辑路径需带 data/ 前缀） */
    static const char *kGridIconMaskImage;
    /** startIndex > endIndex 边界分支（与原 menu_ui 一致） */
    static const char *kBottomOpsHintEmptyRange;
};

#endif
