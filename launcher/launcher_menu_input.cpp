#include "launcher_menu_input.h"
#include "launcher_theme.h"

void LauncherMenuInput::handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm,
                                  bool &quit) {
    if (LauncherTheme::kUseSwitchGameRowLayout) {
        switch (sym) {
        case SDLK_ESCAPE:
        case SDLK_q:
            quit = true;
            break;
        case SDLK_LEFT:
            if (count > 0)
                trySelectUp(selected, count);
            break;
        case SDLK_RIGHT:
            if (count > 0)
                trySelectDown(selected, count);
            break;
        case SDLK_UP:
        case SDLK_DOWN:
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            if (count > 0)
                confirm = true;
            break;
        default:
            break;
        }
    } else {
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
}

void LauncherMenuInput::handleControllerButton(Uint8 button, int &selected, int count,
                                                 bool &confirm, bool &quit) {
    (void)quit;
    if (LauncherTheme::kUseSwitchGameRowLayout) {
        switch (button) {
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_B:
            if (count > 0)
                confirm = true;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            if (count > 0)
                trySelectUp(selected, count);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            if (count > 0)
                trySelectDown(selected, count);
            break;
        default:
            break;
        }
    } else {
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

void LauncherMenuInput::handleJoyAxisMotion(int axis, Sint16 value, int &selected, int count) {
    if (LauncherTheme::kUseSwitchGameRowLayout) {
        if (axis == 0)
            handleControllerAxisLeftX(value, selected, count);
    } else {
        if (axis == 1)
            handleControllerAxisLeftY(value, selected, count);
    }
}

void LauncherMenuInput::handleJoyHatMotion(Uint8 value, int &selected, int count) {
    if (LauncherTheme::kUseSwitchGameRowLayout) {
        if (value & SDL_HAT_LEFT)
            trySelectUp(selected, count);
        else if (value & SDL_HAT_RIGHT)
            trySelectDown(selected, count);
    } else {
        if (value & SDL_HAT_UP)
            trySelectUp(selected, count);
        else if (value & SDL_HAT_DOWN)
            trySelectDown(selected, count);
    }
}

void LauncherMenuInput::handleControllerAxisLeftY(Sint16 value, int &selected, int count) {
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

void LauncherMenuInput::handleControllerAxisLeftX(Sint16 value, int &selected, int count) {
    if (value < -AXIS_DEADZONE) {
        if (leftStickX_ >= -AXIS_DEADZONE)
            trySelectUp(selected, count);
        leftStickX_ = value;
    } else if (value > AXIS_DEADZONE) {
        if (leftStickX_ <= AXIS_DEADZONE)
            trySelectDown(selected, count);
        leftStickX_ = value;
    } else {
        leftStickX_ = 0;
    }
}

void LauncherMenuInput::handleControllerAxisMotion(int axis, Sint16 value, int &selected,
                                                    int count) {
    if (LauncherTheme::kUseSwitchGameRowLayout) {
        if (axis == SDL_CONTROLLER_AXIS_LEFTX)
            handleControllerAxisLeftX(value, selected, count);
    } else {
        if (axis == SDL_CONTROLLER_AXIS_LEFTY)
            handleControllerAxisLeftY(value, selected, count);
    }
}
