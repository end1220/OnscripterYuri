#include "scene_manager.h"

SceneManager::SceneManager(IScene &mainScene, IScene &settingsScene)
    : mainScene_(&mainScene), settingsScene_(&settingsScene), currentScene_(&mainScene) {
    currentScene_->onEnter();
}

IScene &SceneManager::currentScene() {
    return *currentScene_;
}

void SceneManager::switchTo(LauncherSceneTarget target) {
    IScene *next = nullptr;
    if (target == LauncherSceneTarget::kMain)
        next = mainScene_;
    else if (target == LauncherSceneTarget::kSettings)
        next = settingsScene_;

    if (!next || next == currentScene_)
        return;

    currentScene_ = next;
    currentScene_->onEnter();
}
