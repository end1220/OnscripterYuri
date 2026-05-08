#ifndef LAUNCHER_SCENE_MANAGER_H
#define LAUNCHER_SCENE_MANAGER_H

#include "scenes/i_scene.h"

class SceneManager {
public:
    SceneManager(IScene &mainScene, IScene &settingsScene);

    IScene &currentScene();
    void switchTo(LauncherSceneTarget target);

private:
    IScene *mainScene_ = nullptr;
    IScene *settingsScene_ = nullptr;
    IScene *currentScene_ = nullptr;
};

#endif
