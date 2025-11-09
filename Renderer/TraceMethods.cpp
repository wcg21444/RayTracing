#include "TraceMethods.hpp"
#include "Pass.hpp"
#include <future>
#include <stdexcept>
#include <iostream>
#include <shared_mutex>
#include <algorithm>
#include <memory>

#include "Profiler.hpp"

// TraceSdSceneGPU
TraceSdSceneGPU::TraceSdSceneGPU(SdSceneGPUContext &context)
    : DIContext(context),
      traceRenderTarget(1, 1),
      traceShader("GLSL/screenQuad.vs", "GLSL/simpleRayTrace.fs") {}

void TraceSdSceneGPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount)
{
    std::shared_lock<std::shared_mutex> contextLock(*DIContext.snapshotMutex); // read lock

    traceRenderTarget.bind();
    traceRenderTarget.attachColorTexture2D(traceOutput.ID, GL_COLOR_ATTACHMENT0);
    traceRenderTarget.enableColorAttachments();
    traceRenderTarget.resize(traceInput.Width, traceInput.Height);
    traceRenderTarget.setViewport();
    traceShader.use();
    traceShader.setTextureAuto(traceInput.ID, GL_TEXTURE_2D, 0, "lastSample");
    traceShader.setUniform("width", traceInput.Width);
    traceShader.setUniform("height", traceInput.Height);
    float rand = Random::randomFloats(Random::generator);
    traceShader.setUniform("rand", rand);
    traceShader.setUniform("samplesCount", sampleCount);
    DIContext.camSnapshot->setToFragShader(traceShader, "cam");
    // DIContext.cameraRef.setToFragShader(traceShader, "cam");
    {
        std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneBundleRenderingMutex);
        auto &[NodeStorageTexRendering, TriangleStorageTexRendering, SceneRootIndexRendering] = *DIContext.sceneBundleRendering;
        traceShader.setTextureAuto(NodeStorageTexRendering.ID, GL_TEXTURE_2D, 0, "nodeStorageTex");
        traceShader.setTextureAuto(TriangleStorageTexRendering.ID, GL_TEXTURE_2D, 0, "triangleStorageTex");
        traceShader.setUniform("sceneRootIndex", static_cast<unsigned int>(SceneRootIndexRendering));
        SkySettings::SetShaderUniforms(traceShader);
        traceShader.setTextureAuto(DIContext.skyboxSnapShot->ID, GL_TEXTURE_CUBE_MAP, 0, "skybox");
        // traceShader.setTextureAuto(DIContext.skyboxCubemapRef.ID, GL_TEXTURE_CUBE_MAP, 0, "skybox");
        DrawQuad();
    }
    traceRenderTarget.unbind();
}

// TraceSdSceneCPU
TraceSdSceneCPU::TraceSdSceneCPU(SdSceneCPUContext &context) : DIContext(context) {}
void TraceSdSceneCPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount)
{

    traceImageData.resize(traceInput.Width, traceInput.Height);

    auto shade = [this, sampleCount](CPUImageData &imageData, size_t x, size_t y)
    {
        // static thread_local auto &timeStats = Profiler::ThreadStatsAggregator::RegisterTimeStats("Shade");
        // volatile Profiler::ThreadScopedTimer timer(timeStats); // 约15ms开销. 空函数情况下他在这里能记录5ms 这部分是谁的?还是说是精度问题? .函数体开销为1ms
        static thread_local auto &timeSamplerStats = Profiler::ThreadStatsAggregator::RegisterTimeStats("Shade per 100 TimeSamples");
        volatile Profiler::ThreadScopedTimeSampler timeSampler(timeSamplerStats, 100);
        // 构造析构开销也不小 而且他统计不到函数内的递归调用时间.
        //  有没有可能是因为,每一次累积的时间非常少,导致精度累积误差过大?所以与实际时间销毁偏差很大?
        const float perturbStrength = 0.001f;
        auto &pixelColor = imageData.pixelAt(x, y);
        auto uv = imageData.uvAt(x, y);

        Ray ray(
            DIContext.cameraRef.position,
            DIContext.cameraRef.getRayDirection(uv) + Random::RandomVector(perturbStrength));
        std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneRenderingMutex);
        if (!DIContext.sceneRendering)
        {
            throw std::runtime_error("Scene is not loaded.");
        }
        auto newColor = Trace::CastRay(ray, 0, *DIContext.sceneRendering->pDataStorage);
        // auto newColor = Trace::CastRay(Ray{}, 0, *DIContext.sceneRendering->pDataStorage);//5ms左右构造
        pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount); // 约10ms
    };
    size_t rowsPerThread = traceImageData.height / RenderState::CPUNumThreads;
    for (int i = 0; i < RenderState::CPUNumThreads; ++i)
    {
        size_t startY = i * rowsPerThread;
        size_t endY = (i == RenderState::CPUNumThreads - 1) ? traceImageData.height : startY + rowsPerThread;
        this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY, shade]()
                                                  {
            Profiler::AggregatorGuard threadCounterGuard;
            volatile int i =0;
            for (size_t y = startY; y < endY; ++y) {
                for (size_t x = 0; x < traceImageData.width; ++x) {
                    shade(this->traceImageData, x, y);
                    // ++i;
                }
            } }));
    }
    if (!shadingFutures.empty())
    {
        int futureIndex = 0;
        for (auto &future : shadingFutures)
        {

            future.get();
        }
        this->shadingFutures.clear();
    }
    {
        Profiler::ScopedTimeBlock timer("Renderer::render - Trace Copy Data CPU to GPU");
        traceOutput.setData(traceImageData.data());
    }
}

// TraceImSceneCPU
TraceImSceneCPU::TraceImSceneCPU(ImSceneCPUContext &context) : DIContext(context) {}
void TraceImSceneCPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount)
{
    traceImageData.resize(traceInput.Width, traceInput.Height);
    auto shade = [this, sampleCount](CPUImageData &imageData, size_t x, size_t y)
    {
        const float perturbStrength = 0.001f;
        auto &pixelColor = imageData.pixelAt(x, y);
        auto uv = imageData.uvAt(x, y);
        Ray ray(
            DIContext.cameraRef.position,
            DIContext.cameraRef.getRayDirection(uv) + Random::RandomVector(perturbStrength));
        if (!DIContext.sceneRendering)
        {
            throw std::runtime_error("Scene is not loaded.");
        }
        std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneRenderingMutex);
        auto newColor = Trace::CastRay(ray, 0, *DIContext.sceneRendering);
        pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
    };
    size_t rowsPerThread = traceImageData.height / RenderState::CPUNumThreads;
    for (int i = 0; i < RenderState::CPUNumThreads; ++i)
    {
        size_t startY = i * rowsPerThread;
        size_t endY = (i == RenderState::CPUNumThreads - 1) ? traceImageData.height : startY + rowsPerThread;
        this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY, shade]()
                                                  {
            for (size_t y = startY; y < endY; ++y) {
                for (size_t x = 0; x < traceImageData.width; ++x) {
                    shade(this->traceImageData, x, y);
                }
            } }));
    }
    if (!shadingFutures.empty())
    {
        for (auto &future : shadingFutures)
        {
            future.get();
        }
        this->shadingFutures.clear();
    }
    traceOutput.setData(traceImageData.data());
}
