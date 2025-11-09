#include <array>
#include <iostream>

#include "BVHUI.hpp"
#include "Scene.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "DebugObjectRenderer.hpp"
#include "InputHandler.hpp"
#include "Materials/Lambertian.hpp"
#include "ModelLoader.hpp"
#include "Storage.hpp"
#include "UI.hpp"
#include "Renderer.hpp"
#include "Profiler.hpp"
#include "Config.hpp"
#include "RenderState.hpp"

struct Vecs
{
    glm::vec3 vec1;
    glm::vec3 vec2;
    glm::vec3 vec3;
};

int main()
{
    RenderConfig renderConfig("Configs/RenderConfig.json");
    RenderState::LoadConfig(renderConfig);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char *glsl_version = "#version 460";
    GLFWwindow *window = glfwCreateWindow(RenderState::InitWidth, RenderState::InitHeight, "TRayTracing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    InputHandler::BindWindow(window);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //初始关闭鼠标光标

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 字体设置
    float fontSize = 16.f;
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/CONSOLA.TTF", fontSize);
    io.Fonts->Build();

    std::cout << "ImGui Version: " << IMGUI_VERSION << std::endl;

    DebugObjectRenderer::Initialize();
    DebugObjectRenderer::Resize(RenderState::InitWidth, RenderState::InitHeight);

    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
    renderer->resize(RenderState::InitWidth, RenderState::InitHeight);

    SceneConfig ImSceneConfig("Configs/ImSceneConfig.json");
    SceneConfig sceneConfig("Configs/SceneConfig2.json");

    Storage::ImplicitSceneInstance.initialize(ImSceneConfig);
    Storage::SdSceneInstance.initialize(sceneConfig);

    Storage::InitSceneStorageRendering();

    InputHandler::BindToWindowResizeCallback(window, renderer->onResize);
    InputHandler::BindToWindowResizeCallback(window, DebugObjectRenderer::onResize);

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        Profiler::ProcessStatistics();
        Profiler::RenderUI();

        renderer->render();
        renderer->renderUI();

        // 处理 Camera 鼠标交互
        if (ImGui::IsAnyMouseDown() && !ImGui::GetIO().WantCaptureMouse)
        {
            // std::cout << "Mouse Down of Main Window Detected   " << std::endl;
            if (ImGui::IsMouseDown(0)) // Left Button
            {
                if (ImGui::GetIO().KeyAlt) // alt key down
                {
                    auto mousePos = ImGui::GetMousePos();
                    // auto screenPos = ImGui::GetWindowPos(); ///奇怪 是一个固定值 因为开启了Docking, 得到的是OS Coordinates
                    auto viewportPosition = ImGui::GetMainViewport()->Pos;
                    ImVec2 uv = mousePos - viewportPosition;
                    std::cout << "Mouse Position: " << mousePos.x << ", " << mousePos.y << std::endl;
                    std::cout << "Screen Position: " << viewportPosition.x << ", " << viewportPosition.y << std::endl;
                    std::cout << "Launching ray at screen UV: " << uv.x << ", " << uv.y << std::endl;
                    RayVisualizer::LaunchRay(uv, RenderState::CameraInstance); // 相对屏幕坐标
                }
                else if (ImGui::GetIO().KeyShift) // shift + LMB平移摄像机
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0, 0);

                    RenderState::CameraInstance.controller->processPanOffset(mouseDelta.x, mouseDelta.y);
                    RenderState::Dirty |= true;
                    ImGui::ResetMouseDragDelta(0);
                }
                else
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0, 0);
                    RenderState::CameraInstance.controller->processOrientationOffset(mouseDelta.x, -mouseDelta.y);
                    RenderState::Dirty |= true;
                    ImGui::ResetMouseDragDelta(0);
                }
            }
            if (ImGui::IsMouseDown(1)) // Right Button
            {
                if (ImGui::GetIO().KeyShift) // shift + RMB  Zoom FOV
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(1, 0);

                    RenderState::CameraInstance.controller->processZoom(mouseDelta.x, mouseDelta.y);
                    RenderState::Dirty |= true;
                }
                else
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(1, 0);
                    RenderState::CameraInstance.controller->processDollyOffset(mouseDelta.x, mouseDelta.y);
                    RenderState::Dirty |= true;
                }
                ImGui::ResetMouseDragDelta(1);
            }
        }
        float scrollY = ImGui::GetIO().MouseWheel;
        if (scrollY != 0.0f && !ImGui::GetIO().WantCaptureMouse)
        {
            RenderState::CameraInstance.controller->processMouseScroll(scrollY);
        }

        RenderState::CameraInstance.update();

        ImGui::Begin("RenderUI");
        {
            if (ImGui::Button("Add Sphere"))
            {
                RenderState::Dirty |= true;
                RenderState::SceneDirty |= true;
                {
                    std::unique_lock<std::shared_mutex> lock(Storage::ImplicitSceneInstanceMutex);
                    Storage::ImplicitSceneInstance.objects.push_back(std::make_shared<Sphere>(Random::RandomVector(40.f), 8.f, Lambertian(color4(0.7f, 0.3f, 0.3f, 1.0f))));
                    Storage::ImplicitSceneInstance.update();
                }
            }

            ImGui::End();
        }

        SkySettings::RenderUI();

        DebugObjectRenderer::SetCamera(&RenderState::CameraInstance);
        DebugObjectRenderer::Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
    }

    renderer->shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}