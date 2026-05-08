#ifndef LAUNCHER_SCENES_MAIN_SCENE_H
#define LAUNCHER_SCENES_MAIN_SCENE_H

#include "i_scene.h"

#include "../launcher_chrome.h"
#include "../launcher_game_list_view_vertical.h"
#include "../launcher_grid_view.h"
#include "../launcher_menu_input.h"
#include "../launcher_switch_game_row_view.h"

class MainScene : public IScene {
public:
    void onEnter() override;
    void handleEvent(const SDL_Event &event, LauncherSceneInputContext &ctx) override;
    void draw(const LauncherSceneRenderContext &ctx) override;

private:
    void renderMainUi(const LauncherSceneRenderContext &ctx);

    LauncherMenuInput menuInput_;
    VerticalGameListView listView_;
    LauncherGridView gridView_;
    SwitchGameRowView switchRowView_;
    LauncherChrome chrome_;
};

#endif
