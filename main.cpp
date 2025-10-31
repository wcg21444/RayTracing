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
#include "Renderer.hpp"
#include "Storage.hpp"
#include "UI.hpp"
#include "NewRenderer.hpp"

const int InitWidth = 640;
const int InitHeight = 360;

struct Vecs
{
    glm::vec3 vec1;
    glm::vec3 vec2;
    glm::vec3 vec3;
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char *glsl_version = "#version 460";
    GLFWwindow *window = glfwCreateWindow(InitWidth, InitHeight, "TRayTracing", NULL, NULL);
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
    DebugObjectRenderer::Resize(InitWidth, InitHeight);
    std::shared_ptr<Renderer> RTRenderer = std::make_shared<Renderer>();
    RTRenderer->resize(InitWidth, InitHeight);

    std::shared_ptr<NewRenderer> newRenderer = std::make_shared<NewRenderer>();
    newRenderer->resize(InitWidth, InitHeight);

    Storage::OldScene.update();
    ModelLoader::Run(Storage::OldScene);

    // sd::Scene sdscene;

    Storage::SdScene.initialize();
    Storage::InitializeSceneRendering();

    if (Storage::SdSceneLoader.running == false)
    {
        Storage::SdSceneLoader.run(glfwGetCurrentContext()); // running at mainContext
    }

    InputHandler::BindToWindowResizeCallback(window, newRenderer->onResize);
    InputHandler::BindToWindowResizeCallback(window, DebugObjectRenderer::onResize);

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        Profiler::RenderUI();
        // RTRenderer->render(scene);
        // RTRenderer->render(Storage::SdScene);

        newRenderer->render();

        ImGui::Begin("RenderUI");
        {
            if (ImGui::Button("Add Sphere"))
            {
                RenderState::Dirty |= true;
                RenderState::SceneDirty |= true;

                {
                    std::unique_lock<std::shared_mutex> lock(Storage::OldSceneMutex);
                    Storage::OldScene.objects.push_back(std::make_shared<Sphere>(Random::RandomVector(40.f), 8.f, Lambertian(color4(0.7f, 0.3f, 0.3f, 1.0f))));
                    Storage::OldScene.update();
                }
            }
            ImGui::End();
        }
        BVHSettings::RenderUI();

        // BVHSettings::RenderVisualization(scene.BVHTree.root);

        // for (auto&& object : scene.objects) {
        //     if (auto root = object->getInsideBVHRoot()) {
        //         BVHSettings::RenderVisualization(root);
        //     }
        // }

        BVHSettings::RenderVisualization(*Storage::SdScene.pDataStorage);
        SkySettings::RenderUI();

        DebugObjectRenderer::SetCamera(&newRenderer->cam);
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

    // Cleanup
    RTRenderer->shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}