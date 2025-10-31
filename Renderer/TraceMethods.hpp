#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"
#include "SimplifiedData.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "Random.hpp"
#include "UI.hpp"
#include "Trace.hpp"

#include <future>
class TraceSdSceneGPU : public ITraceMethod // 产生对应Context的引用依赖
{
    SdSceneGPUContext &DIContext; // DI 必须
    RenderTarget traceRenderTarget;
    Shader traceShader;
public:
    TraceSdSceneGPU(SdSceneGPUContext &context);
    void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) override;
};

class CPUImageData
{
    std::vector<glm::vec4> pixels; // RGBA format
public:
    size_t width;
    size_t height;
    inline void setPixel(size_t x, size_t y, glm::vec4 &value) { this->pixels[y * width + x] = value; }
    inline glm::vec4 &pixelAt(size_t x, size_t y) { return pixels[y * width + x]; }
    inline glm::vec2 uvAt(size_t x, size_t y) { return glm::vec2(x / float(width), y / float(height)); }
    inline void resize(size_t w, size_t h)
    {
        width = w;
        height = h;
        pixels.resize(w * h, glm::vec4(0.0f));
    }
    inline glm::vec4 *data() { return pixels.data(); }
};

class TraceSdSceneCPU : public ITraceMethod
{
    SdSceneCPUContext &DIContext;
    CPUImageData traceImageData;
    size_t numThreads = 16;
    std::vector<std::future<void>> shadingFutures;
public:
    TraceSdSceneCPU(SdSceneCPUContext &context);
    void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) override;
};

class TraceSceneCPU : public ITraceMethod
{
    SceneCPUContext &DIContext;
    CPUImageData traceImageData;
    size_t numThreads = 16;
    std::vector<std::future<void>> shadingFutures;
public:
    TraceSceneCPU(SceneCPUContext &context);
    void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) override;
};