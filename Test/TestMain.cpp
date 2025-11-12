#include <filesystem>

#include <iostream>
#include <ctime>

#include "Scene.hpp"

#include "Storage.hpp"
#include "Config.hpp"
#include "TestRenderState.hpp"
#include "TestRenderer.hpp"
#include "TestProfiler.hpp"
#include <cstdio>

int main()
{
    freopen("./Test/TestOutput.txt", "a", stdout);//追加而不是覆盖

    std::cout<<"----------------------------------------"<<std::endl;
    std::cout << "Test Working Directory: " << std::filesystem::current_path() << std::endl;
    RenderConfig renderConfig("Test/Configs/TestRenderConfig.json");
    RenderState::Test::LoadConfig(renderConfig);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    const char *glsl_version = "#version 460";
    GLFWwindow *window = glfwCreateWindow(RenderState::InitWidth, RenderState::InitHeight, "TRayTracing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::shared_ptr<TestRenderer> renderer = std::make_shared<TestRenderer>();
    renderer->resize(RenderState::InitWidth, RenderState::InitHeight);

    // SceneConfig sceneConfig("Configs/SceneConfig3_highPoly.json");
    SceneConfig sceneConfig("Test/Configs/TestSceneConfig.json");
    Storage::SdSceneInstance.initialize(sceneConfig);

    Storage::InitSceneStorageRendering();

    if (RenderState::Test::RenderModeString == "GPU_SdScene")
    {
        renderer->changeMode(RenderMode::GPU_SdScene);
    }
    else if (RenderState::Test::RenderModeString == "CPU_SdScene")
    {
        renderer->changeMode(RenderMode::CPU_SdScene);
    }
    std::cout << "Using Render Mode: " << RenderState::Test::RenderModeString << std::endl;

    std::cout << "Scene Initialized, start rendering..." << std::endl;


    renderer->render();

    renderer->shutdown();

    Profiler::Test::OutputStatistics();
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout<<"Current TimeStamps:"<<std::ctime(&now) << std::endl;
    std::cout << "Resolution: " << RenderState::InitWidth << "x" << RenderState::InitHeight << std::endl;
    std::cout << "Sample Times:" << RenderState::Test::SampleTimes << std::endl;
    std::cout << "Render Time:" << Profiler::ConvertNsAuto(Profiler::Test::TimeBlocks["Total Raytracing Time"].duration<std::chrono::nanoseconds>()) << std::endl;
    std::cout << "Sample per Second:" << (RenderState::Test::SampleTimes) / (Profiler::Test::TimeBlocks["Total Raytracing Time"].duration<std::chrono::nanoseconds>().count() / 1e9)
              << std::endl;
    std::cout << "Test Finished." << std::endl;
    std::cout<<"----------------------------------------"<<std::endl;


    glfwTerminate();

    return 0;
}
