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
class Scene;

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
//TODO 改造原来的Renderer类
class NewRenderer
{
private:
    int width = 1600;
    int height = 900;

private:
    RenderMode renderMode = RenderMode::GPU_SdScene;

public:
    std::unique_ptr<ScreenPass> screenPass;
    std::unique_ptr<PostProcessor> postProcessor;
    std::unique_ptr<SkyTexPass> skyTexPass;

    std::unique_ptr<ITracer> tracer;
    std::unique_ptr<IUpLoader> uploader;

    inline static Camera Cam = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));
    Texture2D screenTexture;

    NewRenderer();
    NewRenderer(const NewRenderer &) = delete;
    NewRenderer &operator=(const NewRenderer &) = delete;
    ~NewRenderer();
    void render();
    void resize(int newWidth, int newHeight);
    void shutdown();

private:
    void renderUI();
};