# Linux ARM64 掌机：摇杆模拟鼠标指针与模式切换 — 需求评估与实施方案

## 实施现状（已落地）

以下内容已按当前代码实现（以 `master` 工作树为准）：

- 已新增 `INPUT_MODE_TRADITIONAL` / `INPUT_MODE_POINTER` 两种输入模式，并在引擎内维护逻辑指针坐标与指针参数。
- 模式切换采用**自动策略**：左摇杆超死区自动进入 `INPUT_MODE_POINTER`；连续约 5 秒无左摇杆活动自动回到 `INPUT_MODE_TRADITIONAL`。
- 已实现 `SDL_CONTROLLERAXISMOTION`：GameController 左摇杆可直接驱动逻辑指针，并复用 `mouseMoveEvent` / `mouseOverCheck` 按钮悬停链路。
- 指针模式下 **A 键**（以及 Joystick 路径下映射到 `SDLK_ESCAPE` 的确认键）已改为合成鼠标左键，走 `mousePressEvent`。
- 指针模式下 `SDL_JOYAXISMOTION` 已绕过“有 HAT 就忽略轴”的限制；传统模式保持原有 D-pad/HAT 行为。
- **未实现** Back/L3 手动切换键（与后续用户决策一致）。

### 本次实际代码修改位置（简要）

- `src/onsyuri/ONScripter.h`
  - 增加输入模式与指针相关状态：`input_mode`、`pointer_cursor_x/y`、`pointer_axis_deadzone`、`pointer_axis_max_step`、`last_left_stick_active_ms`、`pointer_idle_timeout_ms`。
- `src/onsyuri/ONScripter.cpp`
  - 在构造函数与 `resetSub()` 初始化上述状态，默认传统模式与 5 秒超时参数。
- `src/onsyuri/ONScripter_event.cpp`
  - 在 `runEventLoop()` 增加左摇杆轴到逻辑指针的更新逻辑。
  - 新增 `SDL_CONTROLLERAXISMOTION` 分支。
  - 在 `SDL_CONTROLLERBUTTONDOWN/UP`、`SDL_JOYBUTTONDOWN/UP` 中加入指针模式点击分支（A/确认键 -> 左键）。
  - 在事件空闲与循环尾部加入“指针模式 5 秒无活动回传统模式”检查。

## 1. 需求摘要

在 **Linux ARM64 掌机**（自带游戏手柄）上运行 OnscripterYuri 引擎时：

| 目标 | 说明 |
|------|------|
| 指针移动 | 用 **左摇杆** 控制游戏画面内与 **ONS 逻辑一致** 的「鼠标」位置（用于 `mouseMoveEvent` / `mouseOverCheck` / 点击判定）。 |
| 点击 | **A 键** 触发 **鼠标左键点击**（等价于在指针位置产生左键按下/抬起）。 |
| 模式切换 | 采用 **自动切换**：左摇杆有活动进入指针模式；连续约 5 秒无左摇杆活动回传统模式。 |

下文先评估合理性，再对照 **仓库现有逻辑**，最后给出可落地的实施步骤。

---

## 2. 需求合理性评估

### 2.1 合理之处

- **ONS 大量流程以「鼠标坐标 + 左键」为核心**（`WAIT_BUTTON_MODE`、`mouseOverCheck`、`mousePressEvent` 等），掌机没有触控板时，用摇杆驱动同一套逻辑，比强行改脚本更一致。
- **模式切换** 有必要：否则摇杆既模拟方向键又模拟指针会冲突；文本快进、系统菜单、脚本里依赖 `SDLK_*` 的行为也需要保留一条「传统」路径。
- **ARM64 Linux** 与桌面 Linux 在 SDL2 事件模型上一致，无需单独抽象「平台」，重点在 **SDL GameController / Joystick** 与 **引擎内事件合成**。

### 2.2 风险与约束

| 风险 | 说明 |
|------|------|
| **「A 键」语义** | SDL 标准里 `SDL_CONTROLLER_BUTTON_A` 是「南键」；不同厂商布局（任天堂/ Xbox）与注释中 *Nintendo-style* 交换有关。实施时需 **以 SDL 枚举为准** 或做可配置映射，并在文档/UI 中写清「物理键位」。 |
| **当前 A 在引擎中的映射** | `transControllerButton` 将 `SDL_CONTROLLER_BUTTON_A` 映射为 **`SDLK_ESCAPE`**（见 `ONScripter_event.cpp`），与「确认/左键」直觉相反。指针模式下应对 **A 走专用分支**，不再走该映射。 |
| **仅 GameController 打开时无摇杆轴处理** | 若设备以 `SDL_GameControllerOpen` 成功、`joystick` 指针为 `NULL`，现有 **`SDL_JOYAXISMOTION` 分支会因 `joystick == NULL` 直接 `break`**，左摇杆不会映射到方向键。指针模式实现时正好要 **统一处理 `SDL_CONTROLLERAXISMOTION`（或从 Controller 取 Joystick 读轴）**，并与传统模式分支清晰隔离。 |
| **带十字键（HAT）的设备** | 现有逻辑在 `SDL_JoystickNumHats(joystick) > 0` 时 **忽略摇杆轴**而只用 HAT。传统模式下这是有意避免冲突；指针模式下若仍要「左摇杆移指针」，需在指针模式中 **绕过该忽略** 或仅对 **左摇杆轴** 单独处理。 |
| **性能与手感** | 摇杆需 **死区、加速曲线、每帧最大位移**，否则指针过飘或过钝；可选「粘滞」到控件中心（未在本次需求中强制）。 |

**结论**：需求在产品与技术上 **可行且合理**，但必须 **显式区分双模式**，并 **修正/旁路** 当前 GameController 下 **无左摇杆轴到游戏逻辑** 的缺口（若目标设备只用 Controller API）。

---

## 3. 现有逻辑梳理（与实现直接相关）

### 3.1 事件入口（`ONScripter_event.cpp`）

- 主循环：`SDL_WaitEventTimeout`，并对手柄按钮做轮询兜底（`pollJoystickButtons`）。
- **键盘**：`SDL_KEYDOWN` / `SDL_KEYUP` → `transKey` → `keyDownEvent` / `keyUpEvent` / `keyPressEvent`。
- **鼠标**：`SDL_MOUSEMOTION` / `SDL_MOUSEBUTTON*` / `SDL_MOUSEWHEEL` → 坐标经 `render_view_rect` / `screen_scale_ratio` 等换算后 → `mouseMoveEvent` / `mousePressEvent`。
- **GameController 按键**：`SDL_CONTROLLERBUTTONDOWN` / `UP` → `transControllerButton` → 与键盘共用同一套处理（**无单独「鼠标左键」分支**）。
- **Joystick 按键**：`SDL_JOYBUTTON*` → `transJoystickButton` → 同上。
- **Joystick 轴**：`SDL_JOYAXISMOTION` 仅在 **`joystick != NULL`** 且 **无 HAT（或当前逻辑下无 HAT）** 时，将左摇杆转为 **方向键按下/抬起**（死区 `ONS_JOY_AXIS_DEADZONE`）。

### 3.2 与「指针」相关的已有能力

- **`mouseMoveEvent`**：用 `event->x` / `event->y` 更新 `current_button_state`，在 `WAIT_BUTTON_MODE` 下调用 `mouseOverCheck`。
- **`mousePressEvent`**：根据 `event->x` / `y` 与 `current_over_button` 产生 `LCLICK` / `S%d` 等脚本可见状态。
- **`warpMouse`**：`SDL_WarpMouseInWindow`，用于脚本 `movemousecursor` 与 `shiftCursorOnButton` 等；**注释明确**：键盘切按钮时不能只依赖 OS 指针，需配合 `mouseOverCheck`。

### 3.3 仓库中 **尚未覆盖** 的点（与本次需求直接相关）

- **`SDL_CONTROLLERAXISMOTION` 未处理**：GameController 模式下左摇杆不会进入现有「轴→方向键」逻辑。
- **指针模式下 A 键**：必须从「映射成 `SDLK_ESCAPE`」改为「合成鼠标左键」或内部等价调用路径。

---

## 4. 实施方案（建议分阶段）

### 4.1 总体结构

1. 在 `ONScripter` 中增加 **输入模式枚举**（示例名）：`INPUT_MODE_TRADITIONAL` / `INPUT_MODE_POINTER`（或 `bool pointer_mode` + 持久化可选）。
2. 增加 **模式切换** 的输入来源（见 4.3），在 **按下边沿** 时切换模式，并可选 **屏上 OSD 提示**（非必须，利于调试）。
3. **`INPUT_MODE_POINTER` 下**：
   - **左摇杆**：将轴读数转为 **游戏坐标系下** 的指针位置更新（见 4.2），调用与真实鼠标一致的路径：`mouseOverCheck` + 更新 `current_button_state` / 必要时 `flush` 或依赖现有重绘逻辑。
   - **A 键**：不调用 `transControllerButton` 的默认映射，改为 **合成左键**：在 **当前逻辑指针坐标** 上构造与 `mousePressEvent` 一致的效果（或推送合成 `SDL_MOUSEBUTTONDOWN`/`UP` 并走同一缩放路径，二选一，优先 **直接调用已有处理函数** 以免重复坐标换算）。
4. **`INPUT_MODE_TRADITIONAL` 下**：保持现有行为；并视情况 **补全** `SDL_CONTROLLERAXISMOTION` → 方向键（与现有 `JOYAXISMOTION` 行为对齐），以便 **仅 Controller** 设备在传统模式下也能用十字键/摇杆当方向键（可单独 PR，但与指针模式同属「手柄完善」）。

### 4.2 左摇杆 → 指针位置（建议算法）

- 读取 **左摇杆 X/Y**（优先 `SDL_CONTROLLERAXISMOTION` 的 `SDL_CONTROLLER_AXIS_LEFTX` / `LEFTY`；若仍走 Joystick API，则 axis 0/1）。
- 应用 **死区**（可与 `ONS_JOY_AXIS_DEADZONE` 同量级或略小，指针模式单独可调）。
- 将超出死区的值转为 **速度向量**（建议线性或轻度加速），每帧或每次事件累积：
  - 新 `x = clamp(旧 x + Δx, 0, screen_width-1)`（**脚本/游戏坐标**，与 `mouseOverCheck` 一致；若引擎内部统一用 device 坐标，则与 `SDL_MOUSEMOTION` 分支中的换算保持一致）。
- **不要** 依赖 `SDL_WarpMouseInWindow` 作为唯一更新手段（部分环境下会额外产生事件或节流）；**推荐** 直接维护「逻辑鼠标坐标」并调用 `mouseOverCheck` + 更新 `current_button_state`，与 `shiftCursorOnButton` 的设计思想一致。
- 若游戏内仍有 **绘制层光标**（`cursor_info`），指针移动时是否同步 **warp 系统光标** 可选（掌机常无可见系统光标，可省略）。

### 4.3 模式切换键（规划，当前版本未采用）

需在 **GameController** 与 **仅 Joystick** 两条路径上都能识别（或统一用键盘键，若掌机映射出键盘事件）。

| 候选 | 说明 |
|------|------|
| **Back（SELECT）** | `SDL_CONTROLLER_BUTTON_BACK` → 当前映射为 `SDLK_0`，冲突风险需评估；适合作为「切换」若脚本很少用 `0`。 |
| **Guide** | `SDL_CONTROLLER_BUTTON_GUIDE` → 映射 `SDLK_F10`，常与系统键冲突，慎用。 |
| **L3 / R3** | 摇杆按下，误触风险；适合「组合键」而非单键。 |
| **L1+R1 组合** | 降低误触，实现略复杂。 |

当前实现未采用手动切换键；保留本节仅作备选设计记录。现版本以“左摇杆活动进入，5 秒无活动退出”的自动策略为准。

### 4.4 A 键 → 左键点击

- 在 `INPUT_MODE_POINTER` 且 **GameController 的 A**（`SDL_CONTROLLER_BUTTON_A`）：
  - 在 **按下** 与 **抬起** 时分别触发与 `SDL_BUTTON_LEFT` 一致的分支（与 `btndown_flag`、长按等现有逻辑兼容）。
- **注意**：若同设备在 **传统模式** 下仍希望 A 为「取消/菜单」（当前 `SDLK_ESCAPE`），切换后必须在指针模式 **完全覆盖** 该语义。

### 4.5 建议改动的文件

| 文件 | 内容 |
|------|------|
| `ONScripter.h` | 模式枚举、状态变量、切换去抖时间戳、指针速度参数（可选）。 |
| `ONScripter_event.cpp` | `SDL_CONTROLLERAXISMOTION` 处理；`SDL_CONTROLLERBUTTON*` / `SDL_JOYBUTTON*` 前增加模式分支；必要时从 `controller` 取 `SDL_GameControllerGetJoystick` 以统一实例 ID（若需与 `JOY*` 事件对照）。 |
| `ONScripter.cpp` | 初始化默认值；可选从配置文件读取。 |

### 4.6 测试建议（Linux ARM64 实机）

- 仅插入掌机手柄：验证 **传统模式** 下方向键/映射键仍可用。
- **指针模式**：摇杆移动时 `WAIT_BUTTON_MODE` 下按钮高亮随动；A 键触发与鼠标左键一致。
- 自动切换：左摇杆活动可稳定进入指针模式；回中后约 5 秒可稳定回传统模式。
- 带 **HAT** 与 **不带 HAT** 的设备各测一台。

### 4.7 不在本次范围（可后续）

- Launcher（`launcher/menu_ui.cpp`）的鼠标点击选游戏（与引擎内指针模式独立）。
- Libretro / Web 前端的键位差异（若有单独 `#ifdef` 需再开任务）。

---

## 5. 小结

| 项 | 结论 |
|----|------|
| 需求是否合理 | **合理**，与 ONS 以鼠标为核心的交互一致；**必须**做模式切换与键位语义隔离。 |
| 现有代码基础 | **鼠标与按钮判定链完整**；**缺口** 主要是 **GameController 左摇杆未进引擎** 以及 **A 键当前映射为 Escape**。 |
| 实施要点 | 增加 **双模式**；指针模式下 **轴→逻辑坐标 + mouseOverCheck**；**A→左键**；采用 **自动切换**（左摇杆活动进入/5 秒无活动退出）；注意 **HAT 与仅 Controller** 两种硬件差异。 |

本文档为实施计划与架构说明；具体 API 命名与参数以编码时与 `ONScripter.h` 现有风格为准。
