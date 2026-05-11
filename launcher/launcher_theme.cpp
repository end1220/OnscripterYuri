#include "launcher_theme.h"

static const SDL_Color kBg = {0, 0, 0, 255};
static const SDL_Color kGameName = {225, 240, 245, 255};
static const SDL_Color kGameNameSel = {66, 225, 205, 255};
static const SDL_Color kItemBg = {0, 235, 205, 0};
static const SDL_Color kItemBgSel = {235, 235, 235, 120};
static const SDL_Color kTitle = {245, 235, 245, 255};
static const SDL_Color kTitleStroke = {0, 90, 120, 255};
static const SDL_Color kEmptyHint = {170, 190, 200, 255};
static const SDL_Color kHintOps = {185, 205, 215, 255};
static const SDL_Color kIconPlaceholder = {200, 230, 235, 255};
static const SDL_Color kSettingsBackdrop = {0, 0, 0, 60};
static const SDL_Color kSettingsPanel = {0, 0, 0, 150};
static const SDL_Color kSettingsPanelBorder = {210, 235, 240, 185};
static const SDL_Color kSettingsButton = {40, 78, 90, 205};
static const SDL_Color kSettingsButtonFocused = {66, 225, 205, 230};
static const SDL_Color kSettingsText = {245, 250, 250, 255};

const SDL_Color &LauncherTheme::bg() { return kBg; }
const SDL_Color &LauncherTheme::gameName() { return kGameName; }
const SDL_Color &LauncherTheme::gameNameSelected() { return kGameNameSel; }
const SDL_Color &LauncherTheme::itemBg() { return kItemBg; }
const SDL_Color &LauncherTheme::itemBgSelected() { return kItemBgSel; }
const SDL_Color &LauncherTheme::titleFill() { return kTitle; }
const SDL_Color &LauncherTheme::titleStroke() { return kTitleStroke; }
const SDL_Color &LauncherTheme::emptyHint() { return kEmptyHint; }
const SDL_Color &LauncherTheme::hintOps() { return kHintOps; }
const SDL_Color &LauncherTheme::iconPlaceholder() { return kIconPlaceholder; }
const SDL_Color &LauncherTheme::settingsBackdrop() { return kSettingsBackdrop; }
const SDL_Color &LauncherTheme::settingsPanel() { return kSettingsPanel; }
const SDL_Color &LauncherTheme::settingsPanelBorder() { return kSettingsPanelBorder; }
const SDL_Color &LauncherTheme::settingsButton() { return kSettingsButton; }
const SDL_Color &LauncherTheme::settingsButtonFocused() { return kSettingsButtonFocused; }
const SDL_Color &LauncherTheme::settingsText() { return kSettingsText; }

const char *LauncherTheme::kTitleText = "ONS游戏";
const char *LauncherTheme::kEmptyListHint = "Roms/ONS目录里未包含ONS游戏哦~";
const char *LauncherTheme::kBottomOpsHintDefault = "Ⓜ菜单  Ⓐ启动 ";
const char *LauncherTheme::kBottomOpsHintGrid = "✚导航  Ⓐ启动  Ⓜ菜单";
const char *LauncherTheme::kSettingsExitText = "退 出";
const char *LauncherTheme::kSettingsReturnText = "返 回";
const char *LauncherTheme::kSettingsAuthorTabText = "作 者";
const char *LauncherTheme::kSettingsAboutTabText = "关 于";
const char *LauncherTheme::kSettingsAuthorNameText = "洛克摸摸鱼";
const char *LauncherTheme::kSettingsAuthorLinkText = "哔哩哔哩 https://b23.tv/3HaIROb";
const char *LauncherTheme::kSettingsHelpTitleText = "ONS启动器2.0.0";
const char *LauncherTheme::kSettingsHelpLine1Text =
    "ONS 游戏请解压至卡1的 Roms/ONS 目录内。";
const char *LauncherTheme::kSettingsHelpLine3Text =
    "ONS 游戏内：Ⓐ 确认；Ⓑ 取消；↑↓键切换按钮/选项；";
const char *LauncherTheme::kSettingsHelpLine3PointerText = "左摇杆可移动光标指针";
const char *LauncherTheme::kSettingsHelpLine5Text =
    "同时按住 Select 与 Start 可强制退出程序。";
const char *LauncherTheme::kSettingsHelpLine6Text =
    "OnscripterYuri · https://github.com/YuriSizuku/OnscripterYuri";
const char *LauncherTheme::kSettingsAboutDeviceBannerText = "Miniloong Pocket1掌机专用";
const char *LauncherTheme::kSettingsSignatureText = "菜 单";
const char *LauncherTheme::kGridSelectedBgImage = "data/cell_light.png";
const char *LauncherTheme::kGridIconMaskImage = "data/icon_mask.png";
const char *LauncherTheme::kBottomOpsHintEmptyRange = "M-设置  A-启动游戏 ";
