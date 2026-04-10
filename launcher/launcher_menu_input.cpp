#include "launcher_menu_input.h"

void LauncherMenuInput::handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm,
                                  bool &quit) {
    switch (sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
        quit = true;
        break;
    case SDLK_UP:
        if (count > 0)
            trySelectUp(selected, count);
        break;
    case SDLK_DOWN:
        if (count > 0)
            trySelectDown(selected, count);
        break;
    case SDLK_RETURN:
    case SDLK_SPACE:
        if (count > 0)
            confirm = true;
        break;
    default:
        break;
    }
}

void LauncherMenuInput::handleControllerButton(Uint8 button, int &selected, int count,
                                               bool &confirm, bool &quit) {
    (void)quit; // 与现版一致：不在此处理退出
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:
    case SDL_CONTROLLER_BUTTON_B:
        if (count > 0)
            confirm = true;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        if (count > 0)
            trySelectUp(selected, count);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        if (count > 0)
            trySelectDown(selected, count);
        break;
    default:
        break;
    }
}

bool LauncherMenuInput::trySelectUp(int &selected, int count) {
    Uint32 now = SDL_GetTicks();
    if (now - lastSelectChangeTicks_ < SELECT_DEBOUNCE_MS)
        return false;
    if (selected > 0)
        --selected;
    else
        selected = count - 1;
    lastSelectChangeTicks_ = now;
    return true;
}

bool LauncherMenuInput::trySelectDown(int &selected, int count) {
    Uint32 now = SDL_GetTicks();
    if (now - lastSelectChangeTicks_ < SELECT_DEBOUNCE_MS)
        return false;
    if (selected < count - 1)
        ++selected;
    else
        selected = 0;
    lastSelectChangeTicks_ = now;
    return true;
}

void LauncherMenuInput::handleJoyAxis(int axis, Sint16 value, int &selected, int count) {
    if (axis != 1)
        return;
    handleControllerAxis(value, selected, count);
}

void LauncherMenuInput::handleJoyHat(Uint8 value, int &selected, int count) {
    if (value & SDL_HAT_UP)
        trySelectUp(selected, count);
    else if (value & SDL_HAT_DOWN)
        trySelectDown(selected, count);
}

void LauncherMenuInput::handleControllerAxis(Sint16 value, int &selected, int count) {
    if (value < -AXIS_DEADZONE) {
        if (leftStickY_ >= -AXIS_DEADZONE)
            trySelectUp(selected, count);
        leftStickY_ = value;
    } else if (value > AXIS_DEADZONE) {
        if (leftStickY_ <= AXIS_DEADZONE)
            trySelectDown(selected, count);
        leftStickY_ = value;
    } else {
        leftStickY_ = 0;
    }
}
