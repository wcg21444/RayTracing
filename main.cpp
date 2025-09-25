#include <array>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Renderer.hpp"
#include "DebugObjectRenderer.hpp"
#include "InputHandler.hpp"
#include "Scene.hpp"

const int InitWidth = 800;
const int InitHeight = 600;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char *glsl_version = "#version 460";
    GLFWwindow *window = glfwCreateWindow(InitWidth, InitHeight, "RayTracing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
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

    InputHandler::BindApplication(RTRenderer);

    Scene::Initialze();
    Scene::Update();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

        RTRenderer->render();

        // RTRenderer->sync();

        // // 遍历bvh 树
        static int maxDepth = 40;
        static int minDepth = 0;
        static bool toggleVisualizeBVH = true;
        ImGui::Begin("BVH Debug");
        {
            ImGui::Text(std::format("ObjectsCount: {}", Scene::Objects.size()).c_str());
            ImGui::DragInt("Max Depth", &maxDepth, 1, 1, 40);
            ImGui::DragInt("Min Depth", &minDepth, 1, 0, 40);
            ImGui::Checkbox("Visualize BVH", &toggleVisualizeBVH);
        }
        ImGui::End();
        std::function<void(BVHNode *, int)> traverse = [&](BVHNode *node, int depth) -> void
        {
            if (!node)
                return;
            if (node->object)
            {
                auto AABB = node->object->getBoundingBox();
                if (toggleVisualizeBVH)
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(1.0f, 0.0f, 0.0f, 1.0f)); });
                }
            }
            else
            {
                auto AABB = node->box;
                if (depth <= maxDepth && depth >= minDepth && toggleVisualizeBVH)
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(0.0f, 1.0f, depth / 8.f, 1.0f)); });
                }
            }
            traverse(node->left, depth + 1);
            traverse(node->right, depth + 1);
        };
        traverse(Scene::BVHTree.root, 0);
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}