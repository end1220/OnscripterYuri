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

const char *LauncherTheme::kTitleText = "ONS游戏";
const char *LauncherTheme::kEmptyListHint = "Roms/ONS目录里未包含ONS游戏哦~";
const char *LauncherTheme::kBottomOpsHintDefault = "Ⓜ退出  Ⓐ启动 ";
const char *LauncherTheme::kBottomOpsHintGrid = "✚导航  Ⓐ启动  Ⓜ退出";
const char *LauncherTheme::kBottomOpsHintEmptyRange = "M-退出  A-启动游戏 ";
