#include "InputHandler.hpp"
#include "Renderer.hpp"
#include "DebugObjectRenderer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// 内部逻辑实现
void InputHandler::ToggleControlMode(GLFWwindow *window)
{
    if (currentMode == APP_CONTROL) // APP TO UI
    {
        // 显示鼠标指针
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // 恢复 UI 模式的鼠标位置
        glfwSetCursorPos(window, mouseState.uiX, mouseState.uiY);
        // ImGui接管输入
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        currentMode = UI_CONTROL;
    }
    else // UI TO APP
    {
        // 隐藏鼠标指针
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // 恢复 APP 模式的虚拟鼠标位置
        glfwSetCursorPos(window, mouseState.appX, mouseState.appY);
        // ImGui忽略输入
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        currentMode = APP_CONTROL;
    }
}

void InputHandler::WindowResizeCallback(GLFWwindow *window, int resizeWidth, int resizeHeight)
{
    DebugObjectRenderer::Resize(resizeWidth, resizeHeight);
    if (pRenderer)
    {
        pRenderer->resize(resizeWidth, resizeHeight);
    }
    else
    {
        throw std::runtime_error("Application not bound to InputHandler.");
    }
}

void InputHandler::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
    {
        ToggleControlMode(window);
        if (currentMode == UI_CONTROL)
        {
            // ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        }
    }
    else
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
}

void InputHandler::MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (currentMode == UI_CONTROL)
    {
        mouseState.uiX = xpos;
        mouseState.uiY = ypos;
    }
    else
    {
        mouseState.appX = xpos;
        mouseState.appY = ypos;
    }
}

void InputHandler::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (currentMode == UI_CONTROL)
    {
        // ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    }
    else
    {
        scrollState.xoffset = xoffset;
        scrollState.yoffset = yoffset;
    }
}

void InputHandler::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (currentMode == UI_CONTROL)
    {
        // ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }
    else
    {
        // 处理程序的鼠标按钮输入
        // ...
        mouseButtonState = {button, action, mods};
    }
}

void InputHandler::BindWindow(GLFWwindow *window)
{
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, WindowResizeCallback);

    // 初始化鼠标指针
    glfwGetCursorPos(window, &mouseState.appX, &mouseState.appY);
    glfwGetCursorPos(window, &mouseState.uiX, &mouseState.uiY);
}

void InputHandler::BindApplication(std::shared_ptr<Renderer> pRenderer)
{
    InputHandler::pRenderer = pRenderer;
}

void InputHandler::ResetInputState()
{
    mouseState = {};
    scrollState = {};
}