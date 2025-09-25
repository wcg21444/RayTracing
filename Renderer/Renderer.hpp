#pragma once

#include "Shader.hpp"

#include "Pass.hpp"
#include "Utils.hpp"
#include "Trace.hpp"
#include "Camera.hpp"
#include "Random.hpp"
#include "UI.hpp"
#include "DebugObjectRenderer.hpp"

#include <thread>
#include <future>

class ScreenPass;
class PostProcessor;
class GPURayTracer;
class SkyTexPass;
class CPURayTracer;

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture2D screenTexture;

private:
    bool useGPU = false;

public:
    inline static bool Dirty = false;// 标记渲染器状态是否需要重置采样 所有更新方法都需要将此变量置true 所有重绘方法都需要检查此变量 并将其置false

public:
    std::unique_ptr<ScreenPass> screenPass;
    std::unique_ptr<PostProcessor> postProcessor;
    std::unique_ptr<GPURayTracer> gpuRayTracer;
    std::unique_ptr<SkyTexPass> skyTexPass;
    std::unique_ptr<CPURayTracer> cpuRayTracer;
    // Camera camera;
    // Scene

    Renderer();
    ~Renderer();
    void render();  
    void resize(int newWidth, int newHeight);
    void resetSamples();

};