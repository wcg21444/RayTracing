#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <memory>
#include <vector>
#include <functional>
#include <atomic>

class Camera;
class Renderer;
using ResizeCallback = std::function<void(int, int)>;
class InputHandler
{
    // 内部状态
private:
    // 控制状态枚举
    enum ControlMode
    {
        APP_CONTROL, // 程序控制模式
        UI_CONTROL,  // ImGui控制模式
    };

    // 存储鼠标位置状态
    struct MouseState
    {
        double uiX, uiY;   // UI 模式下的鼠标位置
        double appX, appY; // APP 模式下的虚拟鼠标位置
    };

    struct ScrollState
    {
        double xoffset, yoffset;
    };

    struct MouseButtonState
    {
        int button;
        int action;
        int mods;
    };

    inline static std::vector<std::weak_ptr<ResizeCallback>> windowResizeCallbacks;

    inline static MouseButtonState mouseButtonState{};
    inline static MouseState mouseState{};
    inline static ScrollState scrollState{};
    inline static ControlMode currentMode = APP_CONTROL;

    // inline static std::shared_ptr<Renderer> pRenderer = nullptr;//InputHandler是更底层的交互模块,不应当依赖上层的渲染模块

private:
    InputHandler() {}
    static void ToggleControlMode(GLFWwindow *window);
    static void WindowResizeCallback(GLFWwindow *window, int resizeWidth, int resizeHeight);
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

public:
    static void BindWindow(GLFWwindow *window);
    
    // 绑定窗口缩放时的回调对象,回调对象应该由缩放入口模块持有.
    static void BindToWindowResizeCallback(GLFWwindow *window, std::shared_ptr<ResizeCallback> callback);
    static void ResetInputState();
};