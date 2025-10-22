#pragma once

#include <thread>
// #include "Shader.hpp"

#include "Pass.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
// #include "UI.hpp"
#include "DebugObjectRenderer.hpp"
// #include "RenderState.hpp"


class ScreenPass;
class PostProcessor;
class GPURayTracer;
class SkyTexPass;
class CPURayTracer;
class Scene;

namespace Storage {
    class SceneLoader;
}

namespace SimplifiedData
{
    class Scene;
}

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture2D screenTexture;

    int CPUThreads = 14;

private:
    bool useGPU = false;

public:
    std::unique_ptr<ScreenPass> screenPass;
    std::unique_ptr<PostProcessor> postProcessor;
    std::unique_ptr<GPURayTracer> gpuRayTracer;
    std::unique_ptr<SkyTexPass> skyTexPass;
    std::unique_ptr<CPURayTracer> cpuRayTracer;
    inline static Camera Cam = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));


    // Camera camera;
    // Scene

    Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    ~Renderer();
    void render(const Scene& scene);
    void render(const SimplifiedData::Scene& scene);
    void resize(int newWidth, int newHeight);
    void resetSamples();
    void shutdown();
private:
    void renderUI();
};

