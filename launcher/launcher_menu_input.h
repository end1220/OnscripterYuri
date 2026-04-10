#ifndef LAUNCHER_MENU_INPUT_H
#define LAUNCHER_MENU_INPUT_H

#include <SDL.h>

/**
 * 启动器菜单输入：键盘、GameController、Joystick；防抖与左摇杆 Y 轴。
 */
class LauncherMenuInput {
public:
    void resetStickState() { leftStickY_ = 0; }

    void handleKey(SDL_Keycode sym, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerButton(Uint8 button, int &selected, int count, bool &confirm, bool &quit);
    void handleControllerAxis(Sint16 value, int &selected, int count);
    void handleJoyAxis(int axis, Sint16 value, int &selected, int count);
    void handleJoyHat(Uint8 value, int &selected, int count);

private:
    bool trySelectUp(int &selected, int count);
    bool trySelectDown(int &selected, int count);

    int leftStickY_ = 0;
    Uint32 lastSelectChangeTicks_ = 0;

    static const int AXIS_DEADZONE = 16000;
    static const Uint32 SELECT_DEBOUNCE_MS = 150;
};

#endif
