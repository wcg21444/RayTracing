#pragma once

#include <thread>
// #include "Shader.hpp"

#include "Pass.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
// #include "UI.hpp"
#include "DebugObjectRenderer.hpp"
// #include "RenderState.hpp"
#include "RenderInterfaces.hpp"

class ScreenPass;
class PostProcessor;
class GPURayTracer;
class SkyTexPass;
class CPURayTracer;
class ImplicitScene;

using ResizeCallback = std::function<void(int, int)>;

namespace Storage
{
    class SceneLoader;
}

namespace SimplifiedData
{
    class Scene;
}

enum class RenderMode
{
    GPU_SdScene,
    GPU_Scene,
    CPU_Scene,
    CPU_SdScene
};
// TODO 改造原来的Renderer类
class Renderer
{
private:
    int width = 1600;
    int height = 900;

private:
    RenderMode renderMode = RenderMode::GPU_SdScene;

public:
    std::unique_ptr<ScreenPass> screenPass = nullptr;
    std::unique_ptr<PostProcessor> postProcessor = nullptr;
    std::unique_ptr<SkyTexPass> skyTexPass = nullptr;

    std::unique_ptr<ITracer> tracer = nullptr;
    std::unique_ptr<IUpLoader> uploader = nullptr;
    std::unique_ptr<IRenderPipeline> currentPipeline = nullptr;

    std::shared_ptr<ResizeCallback> onResize = nullptr;

    Camera cam = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));

    Renderer();
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    ~Renderer();

    void changeMode(RenderMode newMode);
    void render();
    void resize(int newWidth, int newHeight);
    void shutdown();

private:
    void renderUI();
};